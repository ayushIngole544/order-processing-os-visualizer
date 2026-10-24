#!/usr/bin/env bash
# Black-box tests: feed scripted keyboard input to the program and check the output.
# Usage: tests/run_tests.sh [path-to-binary]      (default: ./order_os_visualizer)
# Pattern prefixes:  "!text" = must NOT appear,  "1x:text" = must appear exactly once.

BIN="${1:-./order_os_visualizer}"
pass=0
fail=0

run_case() {
    local name="$1" input="$2"
    shift 2
    local out rc ok=1 msgs=""
    out=$(printf '%b' "$input" | timeout 10 "$BIN" 2>&1)
    rc=$?
    if [ "$rc" -ne 0 ]; then ok=0; msgs+="    exit code $rc (expected 0)"$'\n'; fi
    local pat
    for pat in "$@"; do
        if [[ "$pat" == '!'* ]]; then
            if grep -qF -- "${pat:1}" <<<"$out"; then ok=0; msgs+="    unexpected: ${pat:1}"$'\n'; fi
        elif [[ "$pat" == '1x:'* ]]; then
            if [ "$(grep -cF -- "${pat:3}" <<<"$out")" -ne 1 ]; then ok=0; msgs+="    not exactly once: ${pat:3}"$'\n'; fi
        else
            if ! grep -qF -- "$pat" <<<"$out"; then ok=0; msgs+="    missing: $pat"$'\n'; fi
        fi
    done
    if [ "$ok" -eq 1 ]; then echo "PASS  $name"; pass=$((pass + 1)); else echo "FAIL  $name"; printf '%s' "$msgs"; fail=$((fail + 1)); fi
}

# ---- Startup, menu and input handling ----
first=$(printf '0\n' | timeout 10 "$BIN" | head -1)
if [ "$first" = "==================================================" ]; then echo "PASS  T01a banner is the very first output"; pass=$((pass + 1)); else echo "FAIL  T01a banner first line"; fail=$((fail + 1)); fi
run_case "T01b banner shown once with both authors; menu exit" '0\n' \
    '1x:Made by:' 'Aditya Chafale (24003154)' 'Ayush Ingole (24003155)' 'ORDER PROCESSING OS VISUALIZER' 'Goodbye'
run_case "T02 EOF at main menu exits cleanly (no infinite loop)" '' 'Input stream closed'
run_case "T03 EOF in the middle of a module prompt exits cleanly" '1\n' 'Input stream closed'
run_case "T04 invalid menu input: text, empty, out of range, negative, overflow, spaces" \
    'abc\n\n9\n-1\n99999999999999999999\n 0 \n' \
    "'abc' is not a valid whole number" 'input cannot be empty' 'value must be between 0 and 5' 'Goodbye'

# ---- CPU scheduling (expected values worked out by hand) ----
run_case "T05 FCFS sample: avg WT 9.00, avg TAT 14.20" '1\n2\n1\n0\n0\n' \
    'Average Waiting Time    = 9.00' 'Average Turnaround Time = 14.20' '| O1  | O2  | O3  | O4  | O5  |'
run_case "T06 SJF sample: avg WT 7.20, avg TAT 12.40" '1\n2\n2\n0\n0\n' \
    'Average Waiting Time    = 7.20' 'Average Turnaround Time = 12.40' '| O1  | O4  | O2  | O5  | O3  |'
run_case "T07 Priority sample: avg WT 7.80, avg TAT 13.00" '1\n2\n3\n0\n0\n' \
    'Average Waiting Time    = 7.80' 'Average Turnaround Time = 13.00' '| O1  | O2  | O5  | O4  | O3  |'
run_case "T08 Round Robin q=2 sample: avg WT 13.00, avg TAT 18.20" '1\n2\n4\n2\n0\n0\n' \
    'Average Waiting Time    = 13.00' 'Average Turnaround Time = 18.20' 'time quantum = 2'
run_case "T09 manual entry, invalid values rejected, CPU idle gaps" \
    '1\n1\n0\n2\n4\n0\nabc\n3\n11\n1\n10\n2\n1\n1\n0\n0\n' \
    'value must be between 1 and 15' "'abc' is not a valid whole number" 'value must be between 1 and 10' \
    'Idle' 'Average Waiting Time    = 0.00' 'Average Turnaround Time = 2.50'
run_case "T10 compare all algorithms" '1\n2\n5\n2\n0\n0\n' 'Round Robin q=2' '7.20' '9.00' '13.00'

# ---- Producer-Consumer ----
run_case "T11 slow consumer: producers wait, FIFO kept, semaphores consistent" '2\n2\n1\n1\n5\n2\n0\n' \
    'Total ticks                : 10' 'Producer waits (buffer full)  : 2' 'Consumer waits (buffer empty) : 0' \
    'Peak buffer occupancy      : 2 of 2' 'Processing order           : O1, O2, O3, O4, O5' \
    'FIFO order preserved       : yes' 'consistent (empty = capacity, full = 0, mutex = 1)' 'BLOCKED: wait(empty) fails'
run_case "T12 fast consumers: consumers wait on empty buffer" '2\n3\n1\n2\n4\n1\n0\n' \
    'Total ticks                : 4' 'Consumer waits (buffer empty) : 3' 'Producer waits (buffer full)  : 0' 'BLOCKED: wait(full) fails'
run_case "T13 boundary values (capacity 1, 5+5 threads, 30 orders) and range errors" '2\n0\n11\n1\n5\n5\n30\n5\n0\n' \
    'value must be between 1 and 10' 'Orders produced / consumed : 30 / 30' 'FIFO order preserved       : yes' \
    'consistent (empty = capacity, full = 0, mutex = 1)' '!ERROR'

# ---- Banker's algorithm ----
run_case "T14 sample: Need matrix and safe sequence" '3\n2\n2\n0\n0\n' \
    'Available: [3 3 2]' 'Result: the state is SAFE' 'One safe sequence: O2 -> O4 -> O1 -> O3 -> O5' \
    'Step 1: O2 can finish: Need [1 2 2] <= Work [3 3 2]' 'R1 = Inventory Service (total 10 units)'
run_case "T15 resource requests: granted / must wait / unsafe / exceeds max / empty" \
    '3\n2\n3\n2\n1\n0\n2\n3\n5\n3\n3\n0\n3\n1\n0\n2\n0\n3\n2\n4\n0\n0\n3\n2\n0\n0\n0\n0\n0\n' \
    'GRANTED: the new state is safe' 'WAIT: not enough resources are available right now' \
    'DENIED: granting this request would leave the system in an UNSAFE state' \
    'DENIED: O2 asked for more than its declared maximum claim' 'a request must ask for at least one unit'
run_case "T16 manual unsafe state, Max < Allocation rejected, size limits" \
    '3\n1\n0\n2\n7\n1\n1\n1\n0\n3\n3\n0\n2\n0\n0\n' \
    'value must be between 1 and 10' 'value must be between 1 and 6' 'value must be between 1 and 99' \
    'Result: the state is UNSAFE' '{ O1 O2 }'

# ---- Memory management ----
run_case "T17 First Fit sample (O4 cannot be placed)" '4\n2\n1\n0\n0\n' \
    'Orders allocated        : 4 of 5' 'Internal fragmentation  : 569 KB' \
    'Remaining free memory   : 869 KB (300 KB in empty blocks + 569 KB inside allocated blocks)' \
    'O4 (426 KB): external fragmentation - 869 KB'
run_case "T18 Best Fit sample (all orders fit)" '4\n2\n2\n0\n0\n' \
    'Orders allocated        : 5 of 5' 'Internal fragmentation  : 443 KB' 'All orders were allocated'
run_case "T19 Worst Fit sample" '4\n2\n3\n0\n0\n' 'Orders allocated        : 4 of 5' 'Internal fragmentation  : 769 KB'
run_case "T20 compare all strategies" '4\n2\n4\n0\n0\n' 'First Fit' 'Best Fit' 'Worst Fit' '73.94%' '48.88%'
run_case "T21 manual partitions, invalid sizes rejected, not enough memory" '4\n1\n0\n2\n0\n100\n50\n1\n200\n1\n0\n0\n' \
    'value must be between 1 and 20' 'value must be between 1 and 100000' 'Orders allocated        : 0 of 1' \
    'O1 (200 KB): not enough free memory (150 KB free in total)'

# ---- Integrated demo ----
run_case "T22 integrated: FCFS + First Fit (O4 waits for memory)" '5\n1\n1\n0\n' \
    'STAGE 1' 'STAGE 2' 'STAGE 3' 'STAGE 4' 'PIPELINE SUMMARY' 'Total ticks                : 10' \
    'Producer waits (buffer full)  : 4' 'Result: the state is SAFE' 'Orders given memory               : 4 of 5' \
    'Orders waiting for memory         : O4' 'Average waiting / turnaround      : 6.75 / 12.50'
run_case "T23 integrated: Round Robin q=3 + Best Fit (all orders run)" '5\n4\n3\n2\n0\n' \
    'Orders given memory               : 5 of 5' 'Average Waiting Time    = 12.00' 'Average Turnaround Time = 17.20' '!Orders waiting for memory'

echo
echo "Result: $pass passed, $fail failed"
[ "$fail" -eq 0 ]
