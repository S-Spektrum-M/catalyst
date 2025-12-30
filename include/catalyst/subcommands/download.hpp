#pragma once
#include <CLI/App.hpp>
#include <expected>
#include <utility>

namespace catalyst::download {
struct parse_t {
    std::string git_remote;
    std::string git_branch{""};
    std::filesystem::path target_path;
    std::vector<std::string> profiles;
    std::vector<std::string> enabled_features{};
};

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app);
std::expected<void, std::string> action(const parse_t &);
} // namespace catalyst::download
