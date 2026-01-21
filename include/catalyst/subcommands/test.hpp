#pragma once
#include <CLI/App.hpp>
#include <expected>
#include <string>
#include <vector>
#include "catalyst/workspace.hpp"

namespace catalyst::test {
struct parse_t {
    std::vector<std::string> params;
    std::optional<Workspace> workspace;
};

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app);
std::expected<void, std::string> action(const parse_t &);
} // namespace catalyst::test
