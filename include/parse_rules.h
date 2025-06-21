#pragma once
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

struct base_args_t {};

struct add_args_t {
    std::string dep_name;
    std::string dep_ver;
    std::string dep_src;
    std::vector<std::string> dep_platforms;
    std::vector<std::string> dep_feats;
};

struct init_args_t {
    enum class ProjectType { BINARY, STATIC, SHARED, HEADER };
    static std::map<std::string, init_args_t::ProjectType> type_map; // defined in src/parse_rules.cpp
    std::string proj_name;
    std::string proj_version;
    ProjectType proj_type;
    std::vector<std::string> src_dirs;
    std::vector<std::string> include_dirs;
    std::string build_dir;
    struct {
        std::string CXX;
        std::string CC;
        std::string CXXFLAGS;
        std::string CCFLAGS;
    } tooling;
    bool reinit{false};
};

struct build_args_t {
    std::string profile;
    std::vector<std::string> features{};
};

void parse_rules_add(CLI::App *app, add_args_t &args);
void parse_rules_init(CLI::App *app, init_args_t &args);
void parse_rules_build(CLI::App *app, build_args_t &args);
bool validate_build_args(const build_args_t &args);
