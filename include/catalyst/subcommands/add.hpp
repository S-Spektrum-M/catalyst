#pragma once
#include <CLI/App.hpp>
#include <expected>
#include <string>
#include <vector>

namespace catalyst::add {
struct parse_t {
    std::string name{""};
    std::string version{"latest"};
    std::string source{"catalyst_hub"};
    std::vector<std::string> profiles{{"common"}};
    std::vector<std::string> enabled_features{};
};

namespace git {
struct parse_t {
    std::string name;
    std::string remote;
    std::string version;
    std::vector<std::string> profiles{{"common"}};
    std::vector<std::string> enabled_features{};
};
std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app);
std::expected<void, std::string> action(const parse_t &);
}; // namespace git

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app);
std::expected<void, std::string> action(const parse_t &);
} // namespace catalyst::add
