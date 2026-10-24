#pragma once
#include <string>
#include <vector>

// Sample "sale-day" data shared by all modules (common problem context).
struct SampleOrder {
    std::string name;
    std::string task;
    int arrival;
    int burst;
    int priority;  // lower number = higher priority
    int memoryKB;
};

inline const std::vector<SampleOrder>& sampleOrders() {
    static const std::vector<SampleOrder> orders = {
        {"O1", "Payment", 0, 6, 2, 212},
        {"O2", "Inventory", 1, 4, 1, 417},
        {"O3", "Invoice", 2, 8, 4, 112},
        {"O4", "Delivery", 3, 3, 3, 426},
        {"O5", "Payment", 4, 5, 2, 90},
    };
    return orders;
}

inline const std::vector<int>& sampleMemoryBlocks() {
    static const std::vector<int> blocks = {100, 500, 200, 300, 600};
    return blocks;
}
