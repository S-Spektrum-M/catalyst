#pragma once
#include <CLI/App.hpp>
#include <expected>
#include <string>
#include <vector>

namespace catalyst::build {
struct parse_t {
    bool regen;
    bool force_rebuild;
    bool force_refetch;
    std::vector<std::string> profiles;
    std::vector<std::string> enabled_features;
};

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app);
std::expected<void, std::string> action(const parse_t &);
} // namespace catalyst::build
