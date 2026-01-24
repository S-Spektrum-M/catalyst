#pragma once
#include "catalyst/workspace.hpp"

#include <CLI/App.hpp>
#include <expected>
#include <string>
#include <vector>

namespace catalyst::clean {
struct Parse {
    std::vector<std::string> profiles{"common"};
    std::optional<Workspace> workspace;
};

std::pair<CLI::App *, std::unique_ptr<Parse>> parse(CLI::App &app);
std::expected<void, std::string> action(const Parse &parse_args);
} // namespace catalyst::clean
