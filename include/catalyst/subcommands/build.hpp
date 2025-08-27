#pragma once
#include <CLI/App.hpp>
#include <expected>
#include <string>
#include <vector>

namespace catalyst::build {
struct parse_t {
    bool regen{false};
    bool force_rebuild{false};
    bool force_refetch{false};
    std::vector<std::string> profiles{{"common"}};
    std::vector<std::string> enabled_features{};
};

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app);
std::expected<void, std::string> action(const parse_t &);
} // namespace catalyst::build
