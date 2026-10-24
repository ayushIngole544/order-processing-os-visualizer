#include "scheduler.hpp"

#include <algorithm>
#include <climits>
#include <deque>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <utility>

#include "input.hpp"
#include "sample_data.hpp"
#include "ui.hpp"

namespace {

void addSegment(std::vector<GanttSeg>& gantt, const std::string& label, int start, int end) {
    if (!gantt.empty() && gantt.back().label == label && gantt.back().end == start) {
        gantt.back().end = end;
        return;
    }
    gantt.push_back({label, start, end});
}

// True when task a should run before task b under the given algorithm.
bool runsBefore(const Task& a, const Task& b, SchedAlgo algo) {
    if (algo == SchedAlgo::SJF) {
        if (a.burst != b.burst) return a.burst < b.burst;
    } else if (algo == SchedAlgo::PRIORITY) {
        if (a.priority != b.priority) return a.priority < b.priority;
    }
    return a.arrival < b.arrival;  // FCFS rule, and tie-break for the others
}

void simulateRoundRobin(std::vector<Task>& tasks, int quantum, std::vector<GanttSeg>& gantt) {
    const int n = static_cast<int>(tasks.size());
    std::vector<int> order(static_cast<std::size_t>(n));
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
        return tasks[static_cast<std::size_t>(a)].arrival < tasks[static_cast<std::size_t>(b)].arrival;
    });
    std::vector<int> remaining(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i)
        remaining[static_cast<std::size_t>(i)] = tasks[static_cast<std::size_t>(i)].burst;

    std::deque<int> ready;
    int next = 0, time = 0, finished = 0;
    auto admit = [&]() {
        while (next < n && tasks[static_cast<std::size_t>(order[static_cast<std::size_t>(next)])].arrival <= time) {
            ready.push_back(order[static_cast<std::size_t>(next)]);
            ++next;
        }
    };
    while (finished < n) {
        admit();
        if (ready.empty()) {  // CPU idle until the next order arrives
            const int t = tasks[static_cast<std::size_t>(order[static_cast<std::size_t>(next)])].arrival;
            addSegment(gantt, "Idle", time, t);
            time = t;
            continue;
        }
        const std::size_t i = static_cast<std::size_t>(ready.front());
        ready.pop_front();
        const int run = std::min(quantum, remaining[i]);
        addSegment(gantt, tasks[i].name, time, time + run);
        time += run;
        remaining[i] -= run;
        admit();  // orders that arrived during this slice queue before the preempted one
        if (remaining[i] > 0) {
            ready.push_back(static_cast<int>(i));
        } else {
            tasks[i].completion = time;
            ++finished;
        }
    }
}

void simulateNonPreemptive(std::vector<Task>& tasks, SchedAlgo algo, std::vector<GanttSeg>& gantt) {
    const std::size_t n = tasks.size();
    std::vector<bool> done(n, false);
    int time = 0;
    std::size_t finished = 0;
    while (finished < n) {
        int pick = -1;
        for (std::size_t i = 0; i < n; ++i) {
            if (done[i] || tasks[i].arrival > time) continue;
            if (pick < 0 || runsBefore(tasks[i], tasks[static_cast<std::size_t>(pick)], algo))
                pick = static_cast<int>(i);
        }
        if (pick < 0) {  // nothing has arrived yet: CPU idles
            int nextArrival = INT_MAX;
            for (std::size_t i = 0; i < n; ++i)
                if (!done[i]) nextArrival = std::min(nextArrival, tasks[i].arrival);
            addSegment(gantt, "Idle", time, nextArrival);
            time = nextArrival;
            continue;
        }
        Task& t = tasks[static_cast<std::size_t>(pick)];
        addSegment(gantt, t.name, time, time + t.burst);
        time += t.burst;
        t.completion = time;
        done[static_cast<std::size_t>(pick)] = true;
        ++finished;
    }
}

// Prints the Gantt chart, wrapping to the terminal width.
void printGantt(const std::vector<GanttSeg>& gantt) {
    const std::size_t limit = 76;
    std::string top, bottom;
    int lastEnd = 0;
    auto flush = [&]() {
        if (top.empty()) return;
        std::cout << "  " << top << "|\n  " << bottom << lastEnd << "\n\n";
        top.clear();
        bottom.clear();
    };
    for (const GanttSeg& seg : gantt) {
        const std::string startText = std::to_string(seg.start);
        const std::size_t width = std::max({seg.label.size() + 2, startText.size() + 2, std::size_t(6)});
        if (!top.empty() && top.size() + width + 1 > limit) flush();
        const std::size_t inner = width - 1;
        const std::size_t padLeft = (inner - seg.label.size()) / 2;
        top += "|" + std::string(padLeft, ' ') + seg.label + std::string(inner - seg.label.size() - padLeft, ' ');
        bottom += startText + std::string(width - startText.size(), ' ');
        lastEnd = seg.end;
    }
    flush();
}

std::vector<Task> enterTasks() {
    const int n = input::readInt("Number of orders", 1, 15);
    std::vector<Task> tasks;
    for (int i = 1; i <= n; ++i) {
        Task t;
        t.name = "O" + std::to_string(i);
        t.arrival = input::readInt("  " + t.name + " arrival time", 0, 100);
        t.burst = input::readInt("  " + t.name + " burst (processing) time", 1, 50);
        t.priority = input::readInt("  " + t.name + " priority (1 = highest)", 1, 10);
        tasks.push_back(t);
    }
    return tasks;
}

void printInput(const std::vector<Task>& tasks) {
    std::cout << "\nOrder  Task        Arrival  Burst  Priority\n";
    for (const Task& t : tasks)
        std::cout << std::left << std::setw(7) << t.name << std::setw(12) << t.task << std::right << std::setw(7)
                  << t.arrival << std::setw(7) << t.burst << std::setw(10) << t.priority << "\n";
}

void compareAll(const std::vector<Task>& tasks) {
    const int quantum = input::readInt("Time quantum for Round Robin", 1, 50);
    const SchedAlgo algos[] = {SchedAlgo::FCFS, SchedAlgo::SJF, SchedAlgo::PRIORITY, SchedAlgo::RR};
    std::cout << "\n" << std::left << std::setw(48) << "Algorithm" << std::right << std::setw(10) << "Avg WT"
              << std::setw(10) << "Avg TAT" << "\n";
    for (SchedAlgo a : algos) {
        const SchedResult r = simulate(tasks, a, quantum);
        std::string name = algoName(a);
        if (a == SchedAlgo::RR) name += " q=" + std::to_string(quantum);
        std::cout << std::left << std::setw(48) << name << std::right << std::setw(10) << ui::fmt2(r.avgWaiting)
                  << std::setw(10) << ui::fmt2(r.avgTurnaround) << "\n";
    }
}

}  // namespace

std::string algoName(SchedAlgo algo) {
    switch (algo) {
        case SchedAlgo::FCFS: return "FCFS (First Come First Served)";
        case SchedAlgo::SJF: return "SJF (Shortest Job First, non-preemptive)";
        case SchedAlgo::PRIORITY: return "Priority (non-preemptive, 1 = highest)";
        case SchedAlgo::RR: return "Round Robin";
    }
    return "Unknown";
}

std::vector<Task> sampleTasks() {
    std::vector<Task> tasks;
    for (const SampleOrder& o : sampleOrders()) {
        Task t;
        t.name = o.name;
        t.task = o.task;
        t.arrival = o.arrival;
        t.burst = o.burst;
        t.priority = o.priority;
        tasks.push_back(t);
    }
    return tasks;
}

SchedResult simulate(std::vector<Task> tasks, SchedAlgo algo, int quantum) {
    SchedResult res;
    if (algo == SchedAlgo::RR)
        simulateRoundRobin(tasks, quantum, res.gantt);
    else
        simulateNonPreemptive(tasks, algo, res.gantt);

    double totalWt = 0, totalTat = 0;
    for (Task& t : tasks) {
        t.turnaround = t.completion - t.arrival;
        t.waiting = t.turnaround - t.burst;
        totalWt += t.waiting;
        totalTat += t.turnaround;
    }
    if (!tasks.empty()) {
        res.avgWaiting = totalWt / static_cast<double>(tasks.size());
        res.avgTurnaround = totalTat / static_cast<double>(tasks.size());
    }
    res.tasks = std::move(tasks);
    return res;
}

void printSchedule(const SchedResult& r, SchedAlgo algo, int quantum) {
    std::cout << "\n--- " << algoName(algo);
    if (algo == SchedAlgo::RR) std::cout << " (time quantum = " << quantum << ")";
    std::cout << " ---\n";
    std::cout << std::left << std::setw(7) << "Order" << std::setw(11) << "Task" << std::right << std::setw(8)
              << "Arrival" << std::setw(7) << "Burst" << std::setw(10) << "Priority" << std::setw(12)
              << "Completion" << std::setw(12) << "Turnaround" << std::setw(9) << "Waiting" << "\n";
    for (const Task& t : r.tasks)
        std::cout << std::left << std::setw(7) << t.name << std::setw(11) << t.task << std::right << std::setw(8)
                  << t.arrival << std::setw(7) << t.burst << std::setw(10) << t.priority << std::setw(12)
                  << t.completion << std::setw(12) << t.turnaround << std::setw(9) << t.waiting << "\n";
    std::cout << "\nGantt chart:\n";
    printGantt(r.gantt);
    std::cout << "Turnaround Time = Completion - Arrival;  Waiting Time = Turnaround - Burst\n";
    std::cout << "Average Waiting Time    = " << ui::fmt2(r.avgWaiting) << "\n";
    std::cout << "Average Turnaround Time = " << ui::fmt2(r.avgTurnaround) << "\n";
}

void runSchedulerModule() {
    ui::printSection("CPU SCHEDULING - Order Task Scheduler");
    std::cout << "Orders (payment, inventory, invoice, delivery tasks) compete for the CPU.\n"
              << "1. Enter orders manually\n2. Use sample sale-day orders\n";
    const int source = input::readInt("Choose data source", 1, 2);
    std::vector<Task> tasks = (source == 1) ? enterTasks() : sampleTasks();
    printInput(tasks);

    for (;;) {
        std::cout << "\n1. FCFS\n2. SJF (non-preemptive)\n3. Priority (non-preemptive)\n4. Round Robin\n"
                  << "5. Compare all algorithms\n6. Show input data\n0. Back to main menu\n";
        const int choice = input::readInt("Choose option", 0, 6);
        if (choice == 0) return;
        if (choice == 5) {
            compareAll(tasks);
        } else if (choice == 6) {
            printInput(tasks);
        } else {
            const SchedAlgo algo = static_cast<SchedAlgo>(choice - 1);
            int quantum = 1;
            if (algo == SchedAlgo::RR) quantum = input::readInt("Time quantum", 1, 50);
            printSchedule(simulate(tasks, algo, quantum), algo, quantum);
        }
    }
}
