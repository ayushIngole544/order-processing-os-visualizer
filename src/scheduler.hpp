#pragma once
#include <string>
#include <vector>

enum class SchedAlgo { FCFS, SJF, PRIORITY, RR };

struct Task {
    std::string name;
    std::string task = "Order";
    int arrival = 0;
    int burst = 0;
    int priority = 1;  // lower number = higher priority
    int completion = 0;
    int turnaround = 0;
    int waiting = 0;
};

struct GanttSeg {
    std::string label;
    int start;
    int end;
};

struct SchedResult {
    std::vector<Task> tasks;
    std::vector<GanttSeg> gantt;
    double avgWaiting = 0.0;
    double avgTurnaround = 0.0;
};

std::string algoName(SchedAlgo algo);
std::vector<Task> sampleTasks();
SchedResult simulate(std::vector<Task> tasks, SchedAlgo algo, int quantum);
void printSchedule(const SchedResult& result, SchedAlgo algo, int quantum);
void runSchedulerModule();
