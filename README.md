# Order Processing OS Visualizer

A C++17 command-line **Operating Systems algorithm visualizer** themed around
**Problem No. 2: Online Shopping and Order Processing System** (OS Assignment No. 3 /
Practical No. 09 mini-project, 5th Sem CSE, Section B).

During a big sale an e-commerce platform receives many orders at once. Each order needs CPU time,
memory, and shared services (inventory service, payment gateway, database connections). This program
shows how four classic OS mechanisms keep that workload orderly and safe.

| OS concept | What it does in the order-processing scenario |
|---|---|
| CPU Scheduling | FCFS, SJF, Priority, Round Robin; Gantt chart, waiting and turnaround times |
| Producer-Consumer | Orders enter a bounded buffer (producers); workers take them out (consumers), using semaphores `empty`, `full` and a `mutex` |
| Banker's Algorithm | Allocation / Maximum / Available -> Need matrix, safe sequence, and safe/unsafe handling of new resource requests |
| Memory Management | First / Best / Worst Fit on fixed partitions, memory map, free memory, internal and external fragmentation |

An **integrated demo** (menu option 5) pushes the same five sample orders through all four stages:
intake queue -> Banker's safety check -> memory allocation -> CPU scheduling.

## Requirements
- A C++17 compiler (GCC/G++ 7 or newer; developed and tested with g++ 13.3 on Ubuntu 24.04)
- No external libraries. `make` is optional.

## Build and run
```bash
# Option 1: directly with g++
g++ -std=c++17 -Wall -Wextra -Wpedantic -O2 -o order_os_visualizer src/*.cpp

# Option 2: with make
make

# Run
./order_os_visualizer          # Windows (MinGW): order_os_visualizer.exe
```

## Run the tests
```bash
make test                         # or: ./tests/run_tests.sh ./order_os_visualizer
```
Results of the last full run are in [`docs/TEST_REPORT.md`](docs/TEST_REPORT.md).

## Using the program
The startup banner is shown once, then the main menu:
```
MAIN MENU - Online Shopping and Order Processing System
  1. CPU Scheduling (order task scheduler)
  2. Producer-Consumer (synchronized order queue)
  3. Banker's Algorithm (deadlock avoidance)
  4. Memory Management (order memory allocation)
  5. Integrated demo (all four modules together)
  0. Exit
```
Every module offers **manual entry** or **sample sale-day data**. All input is validated: invalid,
empty or out-of-range entries show an error and ask again. Closing the input stream (Ctrl+D / Ctrl+Z)
exits safely.

### Sample data (used for the examples below)
| Order | Task | Arrival | Burst | Priority (1 = highest) | Memory (KB) |
|---|---|---|---|---|---|
| O1 | Payment | 0 | 6 | 2 | 212 |
| O2 | Inventory | 1 | 4 | 1 | 417 |
| O3 | Invoice | 2 | 8 | 4 | 112 |
| O4 | Delivery | 3 | 3 | 3 | 426 |
| O5 | Payment | 4 | 5 | 2 | 90 |

Memory partitions: 100, 500, 200, 300, 600 KB. Banker's resources: Inventory Service (10 units),
Payment Gateway (5), DB Connections (7).

### Example: SJF (menu 1 -> 2 -> 2)
```
Gantt chart:
  | O1  | O4  | O2  | O5  | O3  |
  0     6     9     13    18    26

Average Waiting Time    = 7.20
Average Turnaround Time = 12.40
```

### Example: Producer-Consumer (menu 2; capacity 2, 1 producer, 1 consumer, 5 orders, consumer every 2 ticks)
```
Tick 4
  P1  BLOCKED: wait(empty) fails, buffer is full
  C1  consumed O2  (wait(full), wait(mutex), remove, signal(mutex), signal(empty))
  Buffer [O3][  ]  in=1 out=0  | empty=1 full=1 mutex=1
```

### Example: Banker's Algorithm (menu 3 -> sample -> safety check)
```
Result: the state is SAFE
One safe sequence: O2 -> O4 -> O1 -> O3 -> O5
```
Menu option 3 inside the module sends a resource request: it is granted only if `Request <= Need`,
`Request <= Available`, and the resulting state is still safe.

### Example: Memory Management (menu 4 -> sample -> First Fit)
```
Block    Size(KB)  Process    Request(KB)   Frag(KB)  Memory map
B1            100  O5                  90         10  [##################..]
B2            500  O1                 212        288  [########............]
B3            200  O3                 112         88  [###########.........]
B4            300  (free)               -          -  [....................]
B5            600  O2                 417        183  [##############......]
```
Best Fit places all five orders with the same data; First Fit leaves O4 waiting. "Compare all
strategies" shows the difference side by side.

## Project structure
```
src/
  main.cpp          menu loop and startup banner
  ui.hpp/.cpp       banner, headings, formatting helpers
  input.hpp/.cpp    validated integer input (handles empty, non-numeric, range, overflow, EOF)
  scheduler.*       CPU scheduling module
  prodcons.*        producer-consumer module
  banker.*          Banker's algorithm module
  memory.*          memory allocation module
  pipeline.*        integrated demo
  sample_data.hpp   shared sample orders and partitions
tests/run_tests.sh  scripted black-box tests
docs/               requirements checklist and test report
```

## Design notes and assumptions
- The PDF does not name a scheduling algorithm, so four are provided. Priority: a lower number means higher priority. SJF and Priority are non-preemptive.
- Producer-consumer is a **deterministic tick-by-tick simulation** of the semaphore solution (not real OS threads), so output is repeatable. A failed `wait()` is shown as BLOCKED and retried next tick.
- Memory uses **fixed partitions**; each partition holds at most one order. Orders that do not fit are reported as waiting.
- The "visualizer" is text-based (ASCII Gantt chart, buffer diagram, memory bars) so it works in any terminal.

## Authors
- Aditya Chafale (24003154)
- Ayush Ingole (24003155)
