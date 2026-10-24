#include "pipeline.hpp"

#include <algorithm>
#include <iostream>

#include "banker.hpp"
#include "input.hpp"
#include "memory.hpp"
#include "prodcons.hpp"
#include "sample_data.hpp"
#include "scheduler.hpp"
#include "ui.hpp"

namespace {

const SampleOrder* findOrder(const std::string& name) {
    for (const SampleOrder& o : sampleOrders())
        if (o.name == name) return &o;
    return nullptr;
}

}  // namespace

void runIntegratedDemo() {
    ui::printSection("INTEGRATED DEMO - Sale-Day Order Pipeline");
    std::cout << "Five customer orders (payment, inventory, invoice, delivery tasks) flow through four stages:\n"
              << "  1. Intake       : producers enqueue orders in a bounded buffer, consumers admit them\n"
              << "  2. Safety       : Banker's Algorithm checks the shared-resource state is safe\n"
              << "  3. Memory       : admitted orders are given memory partitions\n"
              << "  4. CPU          : orders that obtained memory are scheduled on the CPU\n\n";

    std::cout << "Scheduler: 1. FCFS  2. SJF  3. Priority  4. Round Robin\n";
    const SchedAlgo algo = static_cast<SchedAlgo>(input::readInt("Choose scheduling algorithm", 1, 4) - 1);
    const int quantum = (algo == SchedAlgo::RR) ? input::readInt("Time quantum", 1, 50) : 1;
    std::cout << "Memory strategy: 1. First Fit  2. Best Fit  3. Worst Fit\n";
    const FitStrategy fit = static_cast<FitStrategy>(input::readInt("Choose memory strategy", 1, 3) - 1);

    // Stage 1: order intake through the bounded buffer.
    ui::printSection("STAGE 1 - Order intake (Producer-Consumer, buffer = 3, 2 producers, 1 consumer)");
    PCConfig cfg;
    cfg.capacity = 3;
    cfg.producers = 2;
    cfg.consumers = 1;
    cfg.consumerPeriod = 2;
    cfg.totalItems = static_cast<int>(sampleOrders().size());
    for (const SampleOrder& o : sampleOrders()) cfg.labels.push_back(o.name);
    const PCResult pc = runProducerConsumer(cfg, true);
    printPCSummary(cfg, pc);

    // Stage 2: Banker's safety check.
    ui::printSection("STAGE 2 - Resource safety (Banker's Algorithm)");
    const BankerState bank = sampleBankerState();
    printBankerState(bank);
    const SafetyResult safety = checkSafety(bank);
    printSafety(bank, safety);
    if (!safety.safe) {
        std::cout << "\nPipeline stopped: no new work is admitted while the resource state is unsafe.\n";
        return;
    }

    // Stage 3: memory allocation in admission order.
    ui::printSection("STAGE 3 - Memory allocation (" + strategyName(fit) + ")");
    std::vector<MemRequest> reqs;
    for (const std::string& name : pc.consumedOrder) {
        const SampleOrder* o = findOrder(name);
        if (o) reqs.push_back({o->name, o->memoryKB});
    }
    const std::vector<int>& blocks = sampleMemoryBlocks();
    const MemResult mem = allocateMemory(blocks, reqs, fit);
    printMemory(blocks, reqs, mem, fit);

    // Stage 4: CPU scheduling of the orders that obtained memory.
    ui::printSection("STAGE 4 - CPU scheduling (" + algoName(algo) + ")");
    std::vector<Task> runnable;
    std::vector<std::string> waiting;
    const std::vector<Task> all = sampleTasks();
    for (std::size_t r = 0; r < reqs.size(); ++r) {
        if (mem.assignment[r] < 0) {
            waiting.push_back(reqs[r].name);
            continue;
        }
        for (const Task& t : all)
            if (t.name == reqs[r].name) runnable.push_back(t);
    }
    SchedResult sched;
    if (runnable.empty()) {
        std::cout << "No order obtained memory, so nothing can be scheduled.\n";
    } else {
        sched = simulate(runnable, algo, quantum);
        printSchedule(sched, algo, quantum);
    }

    ui::printSection("PIPELINE SUMMARY");
    std::cout << "Orders received through the queue : " << pc.consumedOrder.size() << "\n"
              << "Resource state                    : SAFE\n"
              << "Orders given memory               : " << runnable.size() << " of " << reqs.size() << "\n";
    if (!waiting.empty()) {
        std::cout << "Orders waiting for memory         :";
        for (const std::string& w : waiting) std::cout << " " << w;
        std::cout << "\n";
    }
    if (!runnable.empty())
        std::cout << "Average waiting / turnaround      : " << ui::fmt2(sched.avgWaiting) << " / "
                  << ui::fmt2(sched.avgTurnaround) << "\n";
}
