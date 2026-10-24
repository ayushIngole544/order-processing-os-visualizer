#include <iostream>

#include "banker.hpp"
#include "input.hpp"
#include "memory.hpp"
#include "pipeline.hpp"
#include "prodcons.hpp"
#include "scheduler.hpp"
#include "ui.hpp"

namespace {

void printMenu() {
    std::cout << "\nMAIN MENU - Online Shopping and Order Processing System\n"
              << "  1. CPU Scheduling (order task scheduler)\n"
              << "  2. Producer-Consumer (synchronized order queue)\n"
              << "  3. Banker's Algorithm (deadlock avoidance)\n"
              << "  4. Memory Management (order memory allocation)\n"
              << "  5. Integrated demo (all four modules together)\n"
              << "  0. Exit\n";
}

}  // namespace

int main() {
    ui::printBanner("ORDER PROCESSING OS VISUALIZER");
    try {
        for (;;) {
            printMenu();
            switch (input::readInt("Enter your choice", 0, 5)) {
                case 1: runSchedulerModule(); break;
                case 2: runProducerConsumerModule(); break;
                case 3: runBankerModule(); break;
                case 4: runMemoryModule(); break;
                case 5: runIntegratedDemo(); break;
                default: std::cout << "\nThank you for using the visualizer. Goodbye!\n"; return 0;
            }
        }
    } catch (const InputClosed&) {
        std::cout << "\nInput stream closed. Exiting safely.\n";
    }
    return 0;
}
