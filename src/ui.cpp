#include "ui.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace ui {

void printBanner(const std::string& projectName) {
    const std::size_t width = 50;
    const std::string bar(width, '=');
    const std::size_t pad = projectName.size() < width ? (width - projectName.size()) / 2 : 0;
    std::cout << bar << "\n"
              << std::string(pad, ' ') << projectName << "\n"
              << bar << "\n"
              << "Made by:\n"
              << "Aditya Chafale (24003154)\n"
              << "Ayush Ingole (24003155)\n"
              << bar << "\n";
}

void printSection(const std::string& title) {
    const std::string rule(70, '=');
    std::cout << "\n" << rule << "\n" << title << "\n" << rule << "\n";
}

std::string vecStr(const std::vector<int>& v) {
    std::ostringstream os;
    os << "[";
    for (std::size_t i = 0; i < v.size(); ++i) os << (i ? " " : "") << v[i];
    os << "]";
    return os.str();
}

std::string fmt2(double value) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(2) << value;
    return os.str();
}

}  // namespace ui
