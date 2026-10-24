#include "banker.hpp"

#include <iomanip>
#include <iostream>

#include "input.hpp"
#include "sample_data.hpp"
#include "ui.hpp"

namespace {

bool lessOrEqual(const std::vector<int>& a, const std::vector<int>& b) {
    for (std::size_t i = 0; i < a.size(); ++i)
        if (a[i] > b[i]) return false;
    return true;
}

void printMatrixRow(const std::vector<int>& row) {
    for (int v : row) std::cout << std::setw(4) << v;
    std::cout << "   ";
}

BankerState enterBankerState() {
    BankerState s;
    s.n = input::readInt("Number of order tasks (processes)", 1, 10);
    s.m = input::readInt("Number of resource types", 1, 6);
    const std::size_t n = static_cast<std::size_t>(s.n), m = static_cast<std::size_t>(s.m);
    for (std::size_t i = 0; i < n; ++i) s.pnames.push_back("O" + std::to_string(i + 1));
    for (std::size_t j = 0; j < m; ++j) s.rnames.push_back("Resource " + std::to_string(j + 1));

    std::cout << "\nEnter the Allocation matrix (units currently held):\n";
    s.alloc.assign(n, std::vector<int>(m, 0));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < m; ++j)
            s.alloc[i][j] = input::readInt("  Allocation of R" + std::to_string(j + 1) + " held by " + s.pnames[i], 0, 99);

    std::cout << "\nEnter the Maximum matrix (maximum units each task may ever need):\n";
    s.maxNeed.assign(n, std::vector<int>(m, 0));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < m; ++j)
            s.maxNeed[i][j] = input::readInt("  Maximum of R" + std::to_string(j + 1) + " for " + s.pnames[i] +
                                                 " (must be >= allocation " + std::to_string(s.alloc[i][j]) + ")",
                                             s.alloc[i][j], 99);

    std::cout << "\nEnter the Available vector (free units right now):\n";
    s.avail.assign(m, 0);
    for (std::size_t j = 0; j < m; ++j) s.avail[j] = input::readInt("  Available units of R" + std::to_string(j + 1), 0, 99);
    computeNeed(s);
    return s;
}

void printRequestOutcome(const BankerState& s, RequestStatus status, const std::string& who) {
    switch (status) {
        case RequestStatus::EXCEEDS_MAX:
            std::cout << "\nDENIED: " << who << " asked for more than its declared maximum claim (Request > Need).\n";
            break;
        case RequestStatus::MUST_WAIT:
            std::cout << "\nWAIT: not enough resources are available right now (Request > Available). " << who
                      << " must wait.\n";
            break;
        case RequestStatus::UNSAFE:
            std::cout << "\nDENIED: granting this request would leave the system in an UNSAFE state. " << who
                      << " must wait.\n";
            break;
        case RequestStatus::GRANTED: {
            std::cout << "\nGRANTED: the new state is safe, so the request is approved.\n";
            printBankerState(s);
            printSafety(s, checkSafety(s));
            break;
        }
    }
}

void handleRequest(BankerState& s) {
    const int p = input::readInt("Which task is requesting resources (1 = " + s.pnames.front() + ")", 1, s.n);
    const std::size_t pi = static_cast<std::size_t>(p - 1);
    std::vector<int> req(static_cast<std::size_t>(s.m));
    bool any = false;
    for (std::size_t j = 0; j < req.size(); ++j) {
        req[j] = input::readInt("  Units of R" + std::to_string(j + 1) + " requested by " + s.pnames[pi], 0, 99);
        any = any || req[j] > 0;
    }
    if (!any) {
        std::cout << "\nError: a request must ask for at least one unit.\n";
        return;
    }
    std::cout << "\nRequest " << ui::vecStr(req) << " from " << s.pnames[pi] << "  (Need " << ui::vecStr(s.need[pi])
              << ", Available " << ui::vecStr(s.avail) << ")\n";
    printRequestOutcome(s, requestResources(s, p - 1, req), s.pnames[pi]);
}

}  // namespace

BankerState sampleBankerState() {
    BankerState s;
    s.n = 5;
    s.m = 3;
    for (const SampleOrder& o : sampleOrders()) s.pnames.push_back(o.name);
    s.rnames = {"Inventory Service", "Payment Gateway", "DB Connections"};
    s.alloc = {{0, 1, 0}, {2, 0, 0}, {3, 0, 2}, {2, 1, 1}, {0, 0, 2}};
    s.maxNeed = {{7, 5, 3}, {3, 2, 2}, {9, 0, 2}, {2, 2, 2}, {4, 3, 3}};
    s.avail = {3, 3, 2};
    computeNeed(s);
    return s;
}

void computeNeed(BankerState& s) {
    s.need = s.maxNeed;
    for (std::size_t i = 0; i < s.need.size(); ++i)
        for (std::size_t j = 0; j < s.need[i].size(); ++j) s.need[i][j] = s.maxNeed[i][j] - s.alloc[i][j];
}

SafetyResult checkSafety(const BankerState& s) {
    SafetyResult r;
    std::vector<int> work = s.avail;
    std::vector<bool> finished(static_cast<std::size_t>(s.n), false);
    for (;;) {
        int pick = -1;
        for (std::size_t i = 0; i < finished.size(); ++i)
            if (!finished[i] && lessOrEqual(s.need[i], work)) {
                pick = static_cast<int>(i);
                break;
            }
        if (pick < 0) break;
        const std::size_t pi = static_cast<std::size_t>(pick);
        SafetyStep step{pick, work, work};
        for (std::size_t j = 0; j < work.size(); ++j) work[j] += s.alloc[pi][j];
        step.workAfter = work;
        finished[pi] = true;
        r.sequence.push_back(pick);
        r.steps.push_back(step);
    }
    r.safe = (r.sequence.size() == finished.size());
    for (std::size_t i = 0; i < finished.size(); ++i)
        if (!finished[i]) r.unfinished.push_back(static_cast<int>(i));
    return r;
}

RequestStatus requestResources(BankerState& s, int process, const std::vector<int>& request) {
    const std::size_t pi = static_cast<std::size_t>(process);
    if (!lessOrEqual(request, s.need[pi])) return RequestStatus::EXCEEDS_MAX;
    if (!lessOrEqual(request, s.avail)) return RequestStatus::MUST_WAIT;
    BankerState trial = s;  // pretend to allocate, then test safety
    for (std::size_t j = 0; j < request.size(); ++j) {
        trial.avail[j] -= request[j];
        trial.alloc[pi][j] += request[j];
        trial.need[pi][j] -= request[j];
    }
    if (!checkSafety(trial).safe) return RequestStatus::UNSAFE;
    s = trial;
    return RequestStatus::GRANTED;
}

void printBankerState(const BankerState& s) {
    const std::size_t m = static_cast<std::size_t>(s.m);
    std::cout << "\nResources:\n";
    for (std::size_t j = 0; j < m; ++j) {
        int total = s.avail[j];
        for (const auto& row : s.alloc) total += row[j];
        std::cout << "  R" << j + 1 << " = " << s.rnames[j] << " (total " << total << " units)\n";
    }
    const int block = static_cast<int>(4 * m + 3);
    std::cout << "\n" << std::left << std::setw(9) << "Process" << std::setw(block) << "Allocation" << std::setw(block)
              << "Maximum" << "Need\n" << std::right << std::setw(9) << "";
    for (int k = 0; k < 3; ++k) {
        for (std::size_t j = 0; j < m; ++j) std::cout << std::setw(4) << ("R" + std::to_string(j + 1));
        std::cout << "   ";
    }
    std::cout << "\n";
    for (std::size_t i = 0; i < s.alloc.size(); ++i) {
        std::cout << std::left << std::setw(9) << s.pnames[i] << std::right;
        printMatrixRow(s.alloc[i]);
        printMatrixRow(s.maxNeed[i]);
        printMatrixRow(s.need[i]);
        std::cout << "\n";
    }
    std::cout << "\nAvailable: " << ui::vecStr(s.avail) << "     (Need = Maximum - Allocation)\n";
}

void printSafety(const BankerState& s, const SafetyResult& r) {
    std::cout << "\nSafety algorithm (Work starts as Available = " << ui::vecStr(s.avail) << "):\n";
    int k = 1;
    for (const SafetyStep& st : r.steps) {
        const std::size_t pi = static_cast<std::size_t>(st.process);
        std::cout << "  Step " << k++ << ": " << s.pnames[pi] << " can finish: Need " << ui::vecStr(s.need[pi])
                  << " <= Work " << ui::vecStr(st.workBefore) << "; it releases " << ui::vecStr(s.alloc[pi])
                  << " -> Work " << ui::vecStr(st.workAfter) << "\n";
    }
    if (r.safe) {
        std::cout << "\nResult: the state is SAFE.\nOne safe sequence: ";
        for (std::size_t i = 0; i < r.sequence.size(); ++i)
            std::cout << (i ? " -> " : "") << s.pnames[static_cast<std::size_t>(r.sequence[i])];
        std::cout << "\n";
    } else {
        std::cout << "\nResult: the state is UNSAFE (possible deadlock). No task in { ";
        for (int i : r.unfinished) std::cout << s.pnames[static_cast<std::size_t>(i)] << " ";
        std::cout << "} can get its remaining Need from the resources that would be available.\n";
    }
}

void runBankerModule() {
    ui::printSection("DEADLOCK AVOIDANCE - Banker's Algorithm");
    std::cout << "Order tasks share limited resources (inventory service, payment gateway, DB connections).\n"
              << "1. Enter Allocation / Maximum / Available manually\n2. Use sample data (5 orders, 3 resources)\n";
    const int source = input::readInt("Choose data source", 1, 2);
    BankerState state = (source == 1) ? enterBankerState() : sampleBankerState();
    printBankerState(state);

    for (;;) {
        std::cout << "\n1. Show Allocation / Maximum / Need / Available\n2. Run safety check (find safe sequence)\n"
                  << "3. Make a resource request\n0. Back to main menu\n";
        const int choice = input::readInt("Choose option", 0, 3);
        if (choice == 0) return;
        if (choice == 1)
            printBankerState(state);
        else if (choice == 2)
            printSafety(state, checkSafety(state));
        else
            handleRequest(state);
    }
}
