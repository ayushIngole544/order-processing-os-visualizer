# Test Report

- **Date:** 24 Oct 2026
- **Environment:** Ubuntu 24.04, g++ 13.3.0, `-std=c++17`
- **Build flags used:** `-Wall -Wextra -Wpedantic -O2` (also checked clean with `-Wshadow -Wconversion`)
- **How to reproduce:** `make test` or `./tests/run_tests.sh ./order_os_visualizer`
- **Method:** each case feeds scripted keyboard input to the program and checks the output text.
  Expected scheduling, Banker's and memory values were worked out by hand **before** running the
  program (they match the PDF-style definitions: TAT = Completion - Arrival, WT = TAT - Burst,
  Need = Max - Allocation).

## Results: 24 passed, 0 failed

| ID | Test case | Expected behaviour | Actual | Status |
|---|---|---|---|---|
| T01a | Startup | Banner is the very first line printed | as expected | PASS |
| T01b | Startup / exit | Banner shown exactly once, both names and IDs present, option 0 exits | as expected | PASS |
| T02 | EOF at main menu | Clean exit, no infinite loop | "Input stream closed" | PASS |
| T03 | EOF inside a module | Clean exit | as expected | PASS |
| T04 | Bad menu input: `abc`, empty, `9`, `-1`, 20-digit number, ` 0 ` | Each rejected with a message; padded `0` accepted | as expected | PASS |
| T05 | FCFS, sample orders | Avg WT 9.00, avg TAT 14.20, Gantt O1 O2 O3 O4 O5 | as expected | PASS |
| T06 | SJF, sample orders | Avg WT 7.20, avg TAT 12.40, Gantt O1 O4 O2 O5 O3 | as expected | PASS |
| T07 | Priority, sample orders | Avg WT 7.80, avg TAT 13.00, Gantt O1 O2 O5 O4 O3 | as expected | PASS |
| T08 | Round Robin q=2 | Avg WT 13.00, avg TAT 18.20 | as expected | PASS |
| T09 | Manual orders with bad values, CPU idle gaps | 0 orders, `abc`, burst 0, priority 11 rejected; "Idle" shown; avg WT 0.00, avg TAT 2.50 | as expected | PASS |
| T10 | Compare all algorithms | Table with 9.00 / 7.20 / 7.80 / 13.00 | as expected | PASS |
| T11 | Producer-Consumer, slow consumer | 10 ticks, producer blocked 2x, FIFO kept, semaphores back to initial values | as expected | PASS |
| T12 | Producer-Consumer, fast consumers | 4 ticks, consumers blocked 3x on empty buffer | as expected | PASS |
| T13 | Boundary: capacity 1, 5 producers, 5 consumers, 30 orders; out-of-range capacity | Errors for 0 and 11; all 30 orders processed, FIFO, no error | as expected | PASS |
| T14 | Banker's sample | Available [3 3 2]; first step O2 (Need [1 2 2]); safe sequence O2 -> O4 -> O1 -> O3 -> O5 | as expected | PASS |
| T15 | Banker's requests | Granted; must wait; denied as unsafe; denied as over max claim; empty request rejected | as expected | PASS |
| T16 | Banker's manual, unsafe state | Size limits and Max < Allocation rejected; UNSAFE with { O1 O2 } stuck | as expected | PASS |
| T17 | First Fit, sample | 4 of 5 placed; internal fragmentation 569 KB; O4 reported as fragmentation (869 KB free in total) | as expected | PASS |
| T18 | Best Fit, sample | 5 of 5 placed; internal fragmentation 443 KB | as expected | PASS |
| T19 | Worst Fit, sample | 4 of 5 placed; internal fragmentation 769 KB | as expected | PASS |
| T20 | Compare strategies | Utilization 48.88% (First, Worst), 73.94% (Best) | as expected | PASS |
| T21 | Memory manual, invalid sizes | 0 partitions and size 0 rejected; 200 KB order cannot fit in 150 KB free | as expected | PASS |
| T22 | Integrated, FCFS + First Fit | 4 stages shown; 10 ticks, producer blocked 4x; safe state; O4 waits; avg WT/TAT 6.75 / 12.50 | as expected | PASS |
| T23 | Integrated, RR q=3 + Best Fit | All 5 orders run; avg WT 12.00, avg TAT 17.20 | as expected | PASS |

## Additional checks that were run
- **Compiler warnings:** zero warnings with `-Wall -Wextra -Wpedantic -Wshadow -Wconversion`.
- **Sanitizers:** the full suite was re-run on a build with `-fsanitize=address,undefined`; 24/24 passed with no sanitizer reports.
- **Harness check:** running the suite against a dummy program that prints nothing made all 24 tests fail, so the checks can detect wrong output.

## Not tested / limitations
- Only built and run on Linux with GCC 13. Other compilers and operating systems (Windows, macOS, clang) were **not** tried.
- The producer-consumer module is a deterministic simulation, not real multi-threading, so races between actual threads are not exercised.
- Terminal rendering was checked as plain text only; very narrow terminals (< 80 columns) may wrap the tables.
- GitHub publication has not been attempted (awaiting approval).
