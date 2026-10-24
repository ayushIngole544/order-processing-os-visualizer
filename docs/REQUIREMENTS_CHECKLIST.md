# Requirements-to-Implementation Checklist (Problem No. 2)

Source: *OS Assignment No. 3 and Practical No. 9 (mini project)*, Problem 2 -
Online Shopping and Order Processing System.

| # | Requirement from the PDF | Where implemented | Status |
|---|---|---|---|
| 1 | Manage incoming orders through **synchronized queues** | `prodcons.cpp` - bounded buffer with `empty`/`full` semaphores and `mutex` | Done |
| 2 | **Schedule** processing tasks | `scheduler.cpp` - FCFS, SJF, Priority, Round Robin | Done |
| 3 | CPU scheduling: calculate **waiting time and turnaround time** | `simulate()` / `printSchedule()` - per-order and average values, Gantt chart | Done |
| 4 | **Allocate memory** to concurrent operations | `memory.cpp` - First / Best / Worst Fit on fixed partitions | Done |
| 5 | Memory: **visualize** allocation and remaining free memory | `printMemory()` - table, memory-map bars, remaining free memory | Done |
| 6 | Memory: **analyze fragmentation** | internal fragmentation per block and in total; external fragmentation reported for unallocated orders; strategy comparison | Done |
| 7 | **Prevent unsafe allocation** of shared resources | `banker.cpp` - Banker's Algorithm and request handling | Done |
| 8 | Banker's: use **Allocation, Maximum, Available**; compute **Need** | `enterBankerState()`, `computeNeed()` | Done |
| 9 | Banker's: identify a **safe sequence** when one exists | `checkSafety()` / `printSafety()` with step-by-step Work vector | Done |
| 10 | Producer-Consumer: orders = producers, processing tasks = consumers; bounded buffer / mutex / semaphores | `runProducerConsumer()` | Done |
| 11 | Demonstrate **all four concepts** in the chosen problem | menu options 1-4 | Done |
| 12 | Explain how the four mechanisms **work together** | menu option 5 (`pipeline.cpp`): intake -> safety -> memory -> CPU; also README overview | Done (demo); the written explanation belongs in the case-study report |
| 13 | Modules in the **same visualizer** with a **common problem context** | one executable, shared `sample_data.hpp` | Done |
| 14 | C++ CLI program, startup author banner (user requirement) | `ui::printBanner()` called first in `main()` | Done |
| 15 | Input validation and error recovery (user requirement) | `input::readInt()` and range checks in every module | Done |
| 16 | Source code and documentation on GitHub; link in the Google Form | README, docs/, tests/ prepared | **Pending your approval** - not pushed |
| 17 | Case-study report (max 5 pages, Assignment No. 3) | - | **Not part of this deliverable** (separate document) |

## Note on "existing modules may be reused"
The PDF allows reusing earlier CPU Scheduling and Producer-Consumer modules. Those files were not
provided, so both were written from scratch here.
