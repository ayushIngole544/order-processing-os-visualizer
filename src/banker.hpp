#pragma once
#include <string>
#include <vector>

struct BankerState {
    int n = 0;  // processes (order tasks)
    int m = 0;  // resource types
    std::vector<std::string> pnames;
    std::vector<std::string> rnames;
    std::vector<std::vector<int>> alloc;
    std::vector<std::vector<int>> maxNeed;
    std::vector<std::vector<int>> need;
    std::vector<int> avail;
};

struct SafetyStep {
    int process;
    std::vector<int> workBefore;
    std::vector<int> workAfter;
};

struct SafetyResult {
    bool safe = false;
    std::vector<int> sequence;
    std::vector<SafetyStep> steps;
    std::vector<int> unfinished;
};

enum class RequestStatus { GRANTED, EXCEEDS_MAX, MUST_WAIT, UNSAFE };

BankerState sampleBankerState();
void computeNeed(BankerState& s);
SafetyResult checkSafety(const BankerState& s);
RequestStatus requestResources(BankerState& s, int process, const std::vector<int>& request);
void printBankerState(const BankerState& s);
void printSafety(const BankerState& s, const SafetyResult& r);
void runBankerModule();
