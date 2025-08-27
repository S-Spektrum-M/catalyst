#pragma once
#include <CLI/App.hpp>
#include <expected>
#include <string>
#include <utility>
#include <vector>

namespace catalyst::fetch {
struct parse_t {
    std::vector<std::string> profiles;
};

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app);
std::expected<void, std::string> action(const parse_t &);
}; // namespace catalyst::fetch
