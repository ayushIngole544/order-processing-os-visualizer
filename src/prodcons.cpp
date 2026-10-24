#include "prodcons.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "input.hpp"
#include "ui.hpp"

namespace {

std::string itemLabel(const PCConfig& cfg, int k) {
    const std::size_t idx = static_cast<std::size_t>(k - 1);
    if (idx < cfg.labels.size()) return cfg.labels[idx];
    return "O" + std::to_string(k);
}

std::string bufferLine(const std::vector<std::string>& buf, std::size_t width) {
    std::string s;
    for (const std::string& slot : buf) s += "[" + slot + std::string(width - slot.size(), ' ') + "]";
    return s;
}

std::string joinNames(const std::vector<std::string>& v) {
    std::string s;
    for (std::size_t i = 0; i < v.size(); ++i) s += (i ? ", " : "") + v[i];
    return s;
}

}  // namespace

// Deterministic simulation of the classic semaphore solution:
//   producer: wait(empty); wait(mutex); insert; signal(mutex); signal(full)
//   consumer: wait(full);  wait(mutex); remove; signal(mutex); signal(empty)
// A failed wait() means the thread would sleep, so the attempt is retried next tick.
PCResult runProducerConsumer(const PCConfig& cfg, bool verbose) {
    PCResult res;
    const std::size_t cap = static_cast<std::size_t>(cfg.capacity);
    std::vector<std::string> buffer(cap);
    std::size_t in = 0, out = 0;
    int semEmpty = cfg.capacity, semFull = 0, semMutex = 1;
    int produced = 0, consumed = 0, tick = 0;

    std::size_t labelWidth = 2;
    for (int k = 1; k <= cfg.totalItems; ++k) labelWidth = std::max(labelWidth, itemLabel(cfg, k).size());
    const int maxTicks = cfg.totalItems * (cfg.consumerPeriod + 2) + 50;  // safety net against endless loops

    while (consumed < cfg.totalItems && tick < maxTicks) {
        ++tick;
        std::vector<std::string> events;

        for (int p = 1; p <= cfg.producers && produced < cfg.totalItems; ++p) {
            std::ostringstream ev;
            if (semEmpty == 0) {
                ++res.producerBlocked;
                ev << "P" << p << "  BLOCKED: wait(empty) fails, buffer is full";
            } else {
                --semEmpty;
                --semMutex;  // enter critical section
                const std::string item = itemLabel(cfg, ++produced);
                buffer[in] = item;
                in = (in + 1) % cap;
                res.productionOrder.push_back(item);
                ++semMutex;  // leave critical section
                ++semFull;
                res.maxOccupancy = std::max(res.maxOccupancy, semFull);
                ev << "P" << p << "  produced " << item << "  (wait(empty), wait(mutex), insert, signal(mutex), signal(full))";
            }
            events.push_back(ev.str());
        }

        if (tick % cfg.consumerPeriod == 0) {
            for (int c = 1; c <= cfg.consumers && consumed < cfg.totalItems; ++c) {
                std::ostringstream ev;
                if (semFull == 0) {
                    ++res.consumerBlocked;
                    ev << "C" << c << "  BLOCKED: wait(full) fails, buffer is empty";
                } else {
                    --semFull;
                    --semMutex;
                    const std::string item = buffer[out];
                    buffer[out].clear();
                    out = (out + 1) % cap;
                    res.consumedOrder.push_back(item);
                    ++semMutex;
                    ++semEmpty;
                    ++consumed;
                    ev << "C" << c << "  consumed " << item << "  (wait(full), wait(mutex), remove, signal(mutex), signal(empty))";
                }
                events.push_back(ev.str());
            }
        }

        if (verbose) {
            std::cout << "Tick " << tick << "\n";
            if (events.empty()) std::cout << "  (no action this tick)\n";
            for (const std::string& e : events) std::cout << "  " << e << "\n";
            std::cout << "  Buffer " << bufferLine(buffer, labelWidth) << "  in=" << in << " out=" << out
                      << "  | empty=" << semEmpty << " full=" << semFull << " mutex=" << semMutex << "\n";
        }
    }

    res.ticks = tick;
    res.completed = (consumed == cfg.totalItems);
    res.semaphoresConsistent = res.completed && semEmpty == cfg.capacity && semFull == 0 && semMutex == 1;
    return res;
}

void printPCSummary(const PCConfig& cfg, const PCResult& res) {
    std::cout << "\nSummary\n";
    if (!res.completed) {
        std::cout << "  ERROR: simulation stopped before all orders were processed.\n";
        return;
    }
    std::cout << "  Orders produced / consumed : " << res.productionOrder.size() << " / " << res.consumedOrder.size() << "\n"
              << "  Total ticks                : " << res.ticks << "\n"
              << "  Producer waits (buffer full)  : " << res.producerBlocked << "\n"
              << "  Consumer waits (buffer empty) : " << res.consumerBlocked << "\n"
              << "  Peak buffer occupancy      : " << res.maxOccupancy << " of " << cfg.capacity << "\n"
              << "  Processing order           : " << joinNames(res.consumedOrder) << "\n"
              << "  FIFO order preserved       : " << (res.productionOrder == res.consumedOrder ? "yes" : "no") << "\n"
              << "  Final semaphores           : " << (res.semaphoresConsistent ? "consistent (empty = capacity, full = 0, mutex = 1)" : "INCONSISTENT") << "\n";
}

void runProducerConsumerModule() {
    ui::printSection("PRODUCER-CONSUMER - Synchronized Order Queue");
    std::cout << "Producers = order intake (web/app requests). Consumers = order-processing workers.\n"
              << "A bounded buffer holds waiting orders; semaphores empty/full and a mutex keep it consistent.\n\n";
    PCConfig cfg;
    cfg.capacity = input::readInt("Buffer capacity (order queue slots)", 1, 10);
    cfg.producers = input::readInt("Number of producers", 1, 5);
    cfg.consumers = input::readInt("Number of consumers", 1, 5);
    cfg.totalItems = input::readInt("Total orders to produce", 1, 30);
    cfg.consumerPeriod = input::readInt("Consumer speed: each consumer acts once every N ticks", 1, 5);
    std::cout << "\n";
    const PCResult res = runProducerConsumer(cfg, true);
    printPCSummary(cfg, res);
}
