#include "memory.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>

#include "input.hpp"
#include "sample_data.hpp"
#include "ui.hpp"

namespace {

std::string memoryBar(int size, int used) {
    const int width = 20;
    int filled = used > 0 ? static_cast<int>(std::lround(used * static_cast<double>(width) / size)) : 0;
    if (used > 0 && filled < 1) filled = 1;
    filled = std::min(filled, width);
    return "[" + std::string(static_cast<std::size_t>(filled), '#') + std::string(static_cast<std::size_t>(width - filled), '.') + "]";
}

std::string pct(double value) { return ui::fmt2(value) + "%"; }

void compareStrategies(const std::vector<int>& blocks, const std::vector<MemRequest>& reqs) {
    const FitStrategy all[] = {FitStrategy::FIRST, FitStrategy::BEST, FitStrategy::WORST};
    std::cout << "\n" << std::left << std::setw(12) << "Strategy" << std::right << std::setw(12) << "Allocated"
              << std::setw(14) << "Unallocated" << std::setw(16) << "Internal frag" << std::setw(16)
              << "Empty blocks" << std::setw(13) << "Utilization" << "\n";
    for (FitStrategy s : all) {
        const MemResult r = allocateMemory(blocks, reqs, s);
        std::cout << std::left << std::setw(12) << strategyName(s) << std::right << std::setw(12) << r.allocatedCount
                  << std::setw(14) << (reqs.size() - static_cast<std::size_t>(r.allocatedCount)) << std::setw(13)
                  << r.internalFrag << " KB" << std::setw(13) << r.emptyBlocksFree << " KB" << std::setw(13)
                  << pct(100.0 * r.usedMem / r.totalMem) << "\n";
    }
}

}  // namespace

std::string strategyName(FitStrategy s) {
    switch (s) {
        case FitStrategy::FIRST: return "First Fit";
        case FitStrategy::BEST: return "Best Fit";
        case FitStrategy::WORST: return "Worst Fit";
    }
    return "Unknown";
}

MemResult allocateMemory(const std::vector<int>& blocks, const std::vector<MemRequest>& reqs, FitStrategy strategy) {
    MemResult res;
    res.blockOwner.assign(blocks.size(), -1);
    res.assignment.assign(reqs.size(), -1);
    for (std::size_t r = 0; r < reqs.size(); ++r) {
        int chosen = -1;
        for (std::size_t b = 0; b < blocks.size(); ++b) {
            if (res.blockOwner[b] != -1 || blocks[b] < reqs[r].size) continue;
            if (chosen < 0) {
                chosen = static_cast<int>(b);
                if (strategy == FitStrategy::FIRST) break;
            } else if (strategy == FitStrategy::BEST && blocks[b] < blocks[static_cast<std::size_t>(chosen)]) {
                chosen = static_cast<int>(b);
            } else if (strategy == FitStrategy::WORST && blocks[b] > blocks[static_cast<std::size_t>(chosen)]) {
                chosen = static_cast<int>(b);
            }
        }
        if (chosen >= 0) {
            res.assignment[r] = chosen;
            res.blockOwner[static_cast<std::size_t>(chosen)] = static_cast<int>(r);
            ++res.allocatedCount;
        }
    }
    for (std::size_t b = 0; b < blocks.size(); ++b) {
        res.totalMem += blocks[b];
        if (res.blockOwner[b] < 0) {
            res.emptyBlocksFree += blocks[b];
        } else {
            const int size = reqs[static_cast<std::size_t>(res.blockOwner[b])].size;
            res.usedMem += size;
            res.internalFrag += blocks[b] - size;
        }
    }
    return res;
}

void printMemory(const std::vector<int>& blocks, const std::vector<MemRequest>& reqs, const MemResult& res,
                 FitStrategy strategy) {
    std::cout << "\n--- " << strategyName(strategy) << " (fixed partitions) ---\n";
    std::cout << std::left << std::setw(7) << "Block" << std::right << std::setw(10) << "Size(KB)" << "  " << std::left
              << std::setw(9) << "Process" << std::right << std::setw(13) << "Request(KB)" << std::setw(11)
              << "Frag(KB)" << "  Memory map\n";
    for (std::size_t b = 0; b < blocks.size(); ++b) {
        std::cout << std::left << std::setw(7) << ("B" + std::to_string(b + 1)) << std::right << std::setw(10) << blocks[b] << "  ";
        if (res.blockOwner[b] < 0) {
            std::cout << std::left << std::setw(9) << "(free)" << std::right << std::setw(13) << "-" << std::setw(11) << "-"
                      << "  " << memoryBar(blocks[b], 0) << "\n";
        } else {
            const MemRequest& rq = reqs[static_cast<std::size_t>(res.blockOwner[b])];
            std::cout << std::left << std::setw(9) << rq.name << std::right << std::setw(13) << rq.size << std::setw(11)
                      << (blocks[b] - rq.size) << "  " << memoryBar(blocks[b], rq.size) << "\n";
        }
    }
    std::cout << "('#' = used by the order, '.' = unused space)\n";

    const int totalFree = res.emptyBlocksFree + res.internalFrag;
    int largestEmpty = 0;
    for (std::size_t b = 0; b < blocks.size(); ++b)
        if (res.blockOwner[b] < 0) largestEmpty = std::max(largestEmpty, blocks[b]);

    std::cout << "\nTotal memory            : " << res.totalMem << " KB\n"
              << "Allocated to orders     : " << res.usedMem << " KB (" << pct(100.0 * res.usedMem / res.totalMem) << ")\n"
              << "Remaining free memory   : " << totalFree << " KB (" << res.emptyBlocksFree << " KB in empty blocks + "
              << res.internalFrag << " KB inside allocated blocks)\n"
              << "Internal fragmentation  : " << res.internalFrag << " KB\n"
              << "Orders allocated        : " << res.allocatedCount << " of " << reqs.size() << "\n";

    bool any = false;
    for (std::size_t r = 0; r < reqs.size(); ++r) {
        if (res.assignment[r] >= 0) continue;
        if (!any) std::cout << "\nUnallocated orders (must wait for memory):\n";
        any = true;
        std::cout << "  " << reqs[r].name << " (" << reqs[r].size << " KB): ";
        if (totalFree >= reqs[r].size)
            std::cout << "external fragmentation - " << totalFree << " KB is free in total, but it is split across blocks "
                      << "(largest empty block = " << largestEmpty << " KB)\n";
        else
            std::cout << "not enough free memory (" << totalFree << " KB free in total)\n";
    }
    if (!any) std::cout << "\nAll orders were allocated; no order is waiting for memory.\n";
}

void runMemoryModule() {
    ui::printSection("MEMORY MANAGEMENT - Order Memory Allocation");
    std::cout << "Concurrent order tasks request memory from fixed partitions (sizes in KB).\n"
              << "1. Enter partitions and orders manually\n2. Use sample data\n";
    const int source = input::readInt("Choose data source", 1, 2);

    std::vector<int> blocks;
    std::vector<MemRequest> reqs;
    if (source == 1) {
        const int nb = input::readInt("Number of memory partitions", 1, 20);
        for (int i = 1; i <= nb; ++i) blocks.push_back(input::readInt("  Size of partition B" + std::to_string(i) + " (KB)", 1, 100000));
        const int nr = input::readInt("Number of order tasks needing memory", 1, 20);
        for (int i = 1; i <= nr; ++i) {
            const std::string name = "O" + std::to_string(i);
            reqs.push_back({name, input::readInt("  Memory needed by " + name + " (KB)", 1, 100000)});
        }
    } else {
        blocks = sampleMemoryBlocks();
        for (const SampleOrder& o : sampleOrders()) reqs.push_back({o.name, o.memoryKB});
    }

    std::cout << "\nPartitions (KB):";
    for (int b : blocks) std::cout << " " << b;
    std::cout << "\nRequests (KB)  :";
    for (const MemRequest& r : reqs) std::cout << " " << r.name << "=" << r.size;
    std::cout << "\n";

    for (;;) {
        std::cout << "\n1. First Fit\n2. Best Fit\n3. Worst Fit\n4. Compare all strategies\n0. Back to main menu\n";
        const int choice = input::readInt("Choose option", 0, 4);
        if (choice == 0) return;
        if (choice == 4) {
            compareStrategies(blocks, reqs);
        } else {
            const FitStrategy s = static_cast<FitStrategy>(choice - 1);
            printMemory(blocks, reqs, allocateMemory(blocks, reqs, s), s);
        }
    }
}
