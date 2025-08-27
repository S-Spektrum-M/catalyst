#pragma once
#include <CLI/App.hpp>
#include <string>
#include <vector>

namespace catalyst::clean {
struct parse_t {
    std::vector<std::string> profiles{"common"};
};

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app);
} // namespace catalyst::clean
