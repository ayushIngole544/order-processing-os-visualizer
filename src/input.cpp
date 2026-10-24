#include "input.hpp"

#include <cerrno>
#include <cstdlib>
#include <iostream>

namespace {

std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    const std::size_t b = s.find_first_not_of(ws);
    if (b == std::string::npos) return "";
    const std::size_t e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}

bool parseLong(const std::string& s, long& value) {
    char* end = nullptr;
    errno = 0;
    const long v = std::strtol(s.c_str(), &end, 10);
    if (errno == ERANGE || end == s.c_str() || *end != '\0') return false;
    value = v;
    return true;
}

}  // namespace

namespace input {

int readInt(const std::string& prompt, int minV, int maxV) {
    for (;;) {
        std::cout << prompt << " [" << minV << "-" << maxV << "]: ";
        std::string line;
        if (!std::getline(std::cin, line)) throw InputClosed();
        line = trim(line);
        if (line.empty()) {
            std::cout << "  Error: input cannot be empty. Enter a whole number.\n";
            continue;
        }
        long v = 0;
        if (!parseLong(line, v)) {
            std::cout << "  Error: '" << line << "' is not a valid whole number.\n";
            continue;
        }
        if (v < minV || v > maxV) {
            std::cout << "  Error: value must be between " << minV << " and " << maxV << ".\n";
            continue;
        }
        return static_cast<int>(v);
    }
}

}  // namespace input
