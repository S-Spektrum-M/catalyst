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
void resolve_vcpkg_dependency(const YAML::Node &dep, const std::string &triplet, std::string &ldflags,
                              std::string &ldlibs);
std::expected<void, std::string> resolve_local_dependency(const YAML::Node &dep, std::string &cxxflags,
                                                          std::string &ccflags, std::string &ldflags,
                                                          std::string &ldlibs);
void resolve_pkg_config_dependency(const YAML::Node &dep, std::string &cxxflags, std::string &ccflags,
                                   std::string &ldflags, std::string &ldlibs);
void resolve_system_dependency(const YAML::Node &dep, std::string &cxxflags, std::string &ccflags, std::string &ldflags,
                               std::string &ldlibs);
std::expected<std::string, std::string> lib_path(const YAML::Node &profile);
} // namespace catalyst::generate
