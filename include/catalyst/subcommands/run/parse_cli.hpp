#pragma once
#include <CLI/App.hpp>
#include <string>
#include <vector>

namespace catalyst::run {
struct parse_t {
    std::string profile;
    std::vector<std::string> params{};
};

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app);
} // namespace catalyst::run
