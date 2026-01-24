#pragma once
#include <CLI/App.hpp>
#include <expected>
#include <string>

namespace catalyst::configure {
struct Parse {
    std::string var;
    std::string val;
};

std::pair<CLI::App *, std::unique_ptr<Parse>> parse(CLI::App &app);
std::expected<void, std::string> action(const Parse &);
} // namespace catalyst::configure
