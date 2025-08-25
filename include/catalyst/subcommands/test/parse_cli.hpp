#pragma once
#include <CLI/App.hpp>
#include <string>
#include <vector>

namespace catalyst::test {
struct parse_t {
    std::vector<std::string> params{};
};

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app);
} // namespace catalyst::test
