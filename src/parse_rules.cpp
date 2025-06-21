#include "parse_rules.h"
#include "utils.h"
#include <algorithm>
#include <format>
#include <string>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>

std::map<std::string, init_args_t::ProjectType> init_args_t::type_map = {{"BINARY", init_args_t::ProjectType::BINARY},
                                                                         {"STATIC", init_args_t::ProjectType::STATIC},
                                                                         {"SHARED", init_args_t::ProjectType::SHARED},
                                                                         {"HEADER", init_args_t::ProjectType::HEADER}};

void parse_rules_add(CLI::App *add_sub, add_args_t &args) {
    add_sub->add_option("name", args.dep_name, "The name of the package to add");
    add_sub->add_option("-v,--version", args.dep_ver, "The version to add.")->default_val("latest");
    add_sub->add_option("-s,--src", args.dep_src, "The location to source from.")->default_val("catalyst_hub");
    add_sub->add_option("-p,--profile", args.dep_platforms, "The profile to include for.")
        ->default_val(
            std::vector{"CATALYST_COMMON"}); // NOTE: this is the default since CLI11 errs with an empty string.
    add_sub->add_option("-f,--feature", args.dep_feats, "The features to use.")->default_val(std::vector{"CATALYST_COMMON"});
}

void parse_rules_init(CLI::App *init_sub, init_args_t &args) {
    namespace fs = std::filesystem;

    init_sub->add_option("--name", args.proj_name, "The project's name")
        ->default_val(fs::current_path().filename().string());
    init_sub->add_option("--version", args.proj_version, "The project's version")->default_val("0.0.0");
    init_sub->add_option("-t,--type", args.proj_type, "The build type")
        ->transform(CLI::CheckedTransformer(init_args_t::type_map, CLI::ignore_case))
        ->default_val("BINARY");
    init_sub->add_option("-I,--incdir", args.include_dirs, "The list of directories to look for header files in.")
        ->default_val(std::vector<std::string>{"include"});
    init_sub->add_option("-S,--srcdir", args.src_dirs, "The list of directories to look for source files in.")
        ->default_val(std::vector<std::string>{"src"});
    init_sub->add_option("-B,--builddir", args.build_dir, "The directory to place build objects in.")
        ->default_val("build");
    init_sub->add_option("--CC", args.tooling.CC, "The C compiler to use")->default_val("clang");
    init_sub->add_option("--CXX", args.tooling.CXX, "The C++ compiler to use")->default_val("clang++");

    // NOTE: this is marked static to ensure it outlasts the function scope since CLI11 will caputre the lambda by
    // reference and try and CALL AFTER THE FUNCTION ENDS.
    static auto reinit = [&args](size_t _) { args.reinit = true; };
    init_sub->add_flag("--reinit", reinit);
}

void parse_rules_build(CLI::App *build_sub, build_args_t &args) {
    build_sub->add_option("--profile", args.profile, "The build profile or profile composition to generate for")
        ->default_val("");
    build_sub->add_option("-f,--feature", args.features, "Features to enable.");
}

bool validate_build_args(const build_args_t &args) {
    namespace fs = std::filesystem;
    fs::path profile_file{std::format(
        "catalyst{}{}.yaml",
        [&args]() { // dynamically evaluate wheter or not to do this.
            if (args.profile == "")
                return "";
            else
                return "_";
        }(),
        args.profile)};
    if (args.profile != "" && !fs::exists(profile_file)) {
        log_print(log_level::ERROR, "build profile: {} does not have a corresponding profile specification file: {}.",
                  args.profile, profile_file.string());
        return false;
    }
    if (!args.features.empty()) {
        YAML::Node profile = YAML::LoadFile(profile_file);
        if (!profile["features"].IsDefined()) {
            log_print(log_level::ERROR, "'features' field not defined in profile specification: {}.",
                      profile_file.string());
            return false;
        }
        if (!profile["features"].IsSequence()) {
            log_print(log_level::ERROR, "'features' field not defined as YAML sequence in profile specification: {}.",
                      profile_file.string());
            return false;
        }
        std::vector<std::string> provided_features = profile["features"].as<std::vector<std::string>>();
        for (auto feature : args.features) {
            if (std::find(provided_features.begin(), provided_features.end(), feature) == provided_features.end()) {
                log_print(log_level::ERROR, "feature: {} not defined in profile specification: {}.", feature,
                          profile_file.string());
                return false;
            }
        }
    }
    return true;
}
