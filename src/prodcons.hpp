#pragma once
#include <string>
#include <vector>

struct PCConfig {
    int capacity = 3;        // bounded buffer size
    int producers = 1;       // order-intake threads (producers)
    int consumers = 1;       // order-processing workers (consumers)
    int totalItems = 5;      // orders to produce
    int consumerPeriod = 1;  // consumers act once every N ticks (models slower processing)
    std::vector<std::string> labels;  // optional order names (default O1, O2, ...)
};

struct PCResult {
    std::vector<std::string> productionOrder;
    std::vector<std::string> consumedOrder;
    int ticks = 0;
    int producerBlocked = 0;
    int consumerBlocked = 0;
    int maxOccupancy = 0;
    bool completed = false;
    bool semaphoresConsistent = false;
};

PCResult runProducerConsumer(const PCConfig& cfg, bool verbose);
void printPCSummary(const PCConfig& cfg, const PCResult& res);
void runProducerConsumerModule();
