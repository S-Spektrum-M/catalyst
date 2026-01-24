#pragma once
#include "catalyst/workspace.hpp"

#include <CLI/App.hpp>
#include <expected>
#include <string>
#include <vector>

namespace catalyst::test {
struct Parse {
    std::vector<std::string> params;
    std::optional<Workspace> workspace;
};

std::pair<CLI::App *, std::unique_ptr<Parse>> parse(CLI::App &app);
std::expected<void, std::string> action(const Parse &);
} // namespace catalyst::test
