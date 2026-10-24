#pragma once
#include <string>
#include <vector>

enum class FitStrategy { FIRST, BEST, WORST };

struct MemRequest {
    std::string name;
    int size;  // KB
};

struct MemResult {
    std::vector<int> blockOwner;  // request index stored in each block, or -1 if empty
    std::vector<int> assignment;  // block index chosen for each request, or -1 if unallocated
    int totalMem = 0;
    int usedMem = 0;
    int internalFrag = 0;      // unused space inside allocated blocks
    int emptyBlocksFree = 0;   // total size of blocks holding no request
    int allocatedCount = 0;
};

std::string strategyName(FitStrategy s);
MemResult allocateMemory(const std::vector<int>& blocks, const std::vector<MemRequest>& reqs, FitStrategy strategy);
void printMemory(const std::vector<int>& blocks, const std::vector<MemRequest>& reqs, const MemResult& res,
                 FitStrategy strategy);
void runMemoryModule();
