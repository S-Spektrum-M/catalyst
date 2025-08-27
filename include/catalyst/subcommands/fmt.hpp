#pragma once
#include <CLI/App.hpp>
#include <expected>
#include <string>
#include <vector>

namespace catalyst::fmt {
struct parse_t {
    std::vector<std::string> profiles{"common"};
};

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app);
std::expected<void, std::string> action(const parse_t &parse_args);
} // namespace catalyst::fmt
