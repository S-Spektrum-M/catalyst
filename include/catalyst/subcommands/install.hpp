#pragma once
#include <CLI/App.hpp>
#include <expected>
#include <utility>

namespace catalyst::install {
struct parse_t {
    std::filesystem::path source_path;
    std::filesystem::path target_path;
    std::vector<std::string> profiles;
};

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app);
std::expected<void, std::string> action(const parse_t &);
} // namespace catalyst::install
