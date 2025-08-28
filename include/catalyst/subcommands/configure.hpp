#pragma once
#include <CLI/App.hpp>
#include <expected>
#include <regex>
#include <shared_mutex>
#include <string>
#include <vector>

namespace catalyst::configure {
struct parse_t {
    std::string var;
    std::string val;
};

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app);
std::expected<void, std::string> action(const parse_t &);
} // namespace catalyst::configure
