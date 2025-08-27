#pragma once
#include <CLI/App.hpp>
#include <expected>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace catalyst::generate {
struct parse_t {
    std::vector<std::string> profiles;
    std::vector<std::string> enabled_features;
};

std::expected<YAML::Node, std::string> profile_composition(const std::vector<std::string> &profiles);
std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app);
std::expected<void, std::string> action(const parse_t &);
} // namespace catalyst::generate
