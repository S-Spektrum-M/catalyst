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

struct find_res {
    std::string lib_path;
    std::string inc_path;
    std::string libs;
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

std::expected<find_res, std::string> find_dep(const std::string &build_dir, const YAML::Node &dep);
std::expected<find_res, std::string> find_local(const YAML::Node &dep);
std::expected<find_res, std::string> find_system(const YAML::Node &dep);
std::expected<find_res, std::string> find_vcpkg(const YAML::Node &dep);
std::expected<find_res, std::string> find_git(const std::string &build_dir, const YAML::Node &dep);
} // namespace catalyst::generate
