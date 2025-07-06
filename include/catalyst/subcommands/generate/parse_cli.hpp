#pragma once
#include <CLI/App.hpp>
#include <string>
#include <vector>

namespace catalyst::generate {
struct parse_t {
    std::vector<std::string> profiles;
    std::vector<std::string> enabled_features;
};

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app);
} // namespace catalyst::generate
