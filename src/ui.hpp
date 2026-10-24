#pragma once
#include <string>
#include <vector>

namespace ui {
void printBanner(const std::string& projectName);
void printSection(const std::string& title);
std::string vecStr(const std::vector<int>& v);  // "[3 3 2]"
std::string fmt2(double value);                 // two decimal places
}  // namespace ui
