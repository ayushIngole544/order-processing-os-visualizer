#pragma once
#include <stdexcept>
#include <string>

// Thrown when stdin is closed (EOF) so every prompt loop can terminate safely.
struct InputClosed : std::runtime_error {
    InputClosed() : std::runtime_error("input closed") {}
};

namespace input {
// Repeats the prompt until a whole number in [minV, maxV] is entered.
int readInt(const std::string& prompt, int minV, int maxV);
}  // namespace input
