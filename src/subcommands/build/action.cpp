#include "catalyst/log-utils/log.hpp"
#include <algorithm>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <vector>
#include <yaml-cpp/node/node.h>

#include "catalyst/hooks.hpp"
#include "catalyst/subcommands/build.hpp"
#include "catalyst/subcommands/fetch.hpp"
#include "catalyst/subcommands/generate.hpp"
#include "catalyst/yaml-utils/Configuration.hpp"

namespace catalyst::build {
namespace fs = std::filesystem;

bool dep_missing(const YAML_UTILS::Configuration &config) {
    catalyst::logger.log(LogLevel::INFO, "Checking for missing dependencies.");
    fs::path build_dir = config.get_string("manifest.dirs.build").value_or("build");
    if (!config.has("dependencies")) {
        catalyst::logger.log(LogLevel::INFO, "No dependencies declared, skipping check.");
        return false;
    }
    // TODO: needs to be updated to respect actual dependency types
    if (const auto &pc = config.get_root();
        std::any_of(pc["dependencies"].begin(), pc["dependencies"].end(), [&](const YAML::Node &dep) {
            bool missing = !fs::exists(build_dir / "catalyst-libs" / dep["name"].as<std::string>());
            if (missing) {
                catalyst::logger.log(LogLevel::WARN, "Missing dependency: {}", dep["name"].as<std::string>());
            }
            return missing;
        }))
        return true;
    catalyst::logger.log(LogLevel::INFO, "All dependencies are present.");
    return false;
}

// FIXME: use better redirection scheme
std::expected<void, std::string> generate_compile_commands(const fs::path &build_dir) {
    catalyst::logger.log(LogLevel::INFO, "Generating compile commands database.");
    fs::path real_compdb_path = build_dir / "compile_commands.json";
    std::string compdb_command =
        std::format("ninja -C {} -t compdb > {}", build_dir.string(), real_compdb_path.string());
    catalyst::logger.log(LogLevel::INFO, "Executing command: {}", compdb_command);
    if (std::system(compdb_command.c_str()) != 0) {
        return std::unexpected("failed to generate compile commands");
    }
    return {};
}

std::expected<void, std::string> action(const parse_t &parse_args) {
    catalyst::logger.log(LogLevel::INFO, "Build subcommand invoked.");
    std::vector<std::string> profiles = parse_args.profiles;
    if (std::find(profiles.begin(), profiles.end(), "common") == profiles.end()) {
        profiles.insert(profiles.begin(), "common");
    }

    catalyst::logger.log(LogLevel::INFO, "Composing profiles.");
    YAML_UTILS::Configuration config{profiles};

    catalyst::logger.log(LogLevel::INFO, "Running pre-build hooks.");
    if (auto res = hooks::pre_build(config); !res) {
        catalyst::logger.log(LogLevel::ERROR, "Pre-build hook failed: {}", res.error());
        if (auto hook_res = hooks::on_build_failure(config); !hook_res) {
            catalyst::logger.log(LogLevel::ERROR, "on_build_failure hook failed: {}", hook_res.error());
            return std::unexpected(res.error() +
                                   "\nAdditionally, the on_build_failure hook failed with error: " + hook_res.error());
        }
        return res;
    }

    fs::path build_dir = config.get_string("manifest.dirs.build").value_or("build");
    bool regenerate = true;
    // NOTE: this is just overall a BAD check for whether it changed
    // TODO: Update to actually check values
    //
    // if (fs::exists(build_dir / "profile_composition.yaml")) {
    //     YAML::Node existing_pc = YAML::LoadFile((build_dir / "profile_composition.yaml").string());
    //     if (existing_pc.size() != profile_comp.size()) {
    //         catalyst::logger.log(LogLevel::INFO, "Profile composition changed, regenerating build files.");
    //         regenerate = true;
    //     }
    // }

    // DONE: generate the build file if requested or needed
    if (!fs::exists(build_dir / "build.ninja") || parse_args.regen || regenerate) {
        catalyst::logger.log(LogLevel::INFO, "Generating build files.");
        auto res = catalyst::generate::action({parse_args.profiles, parse_args.enabled_features});
        if (!res) {
            catalyst::logger.log(LogLevel::ERROR, "Failed to generate build files: {}", res.error());
            if (auto hook_res = hooks::on_build_failure(config); !hook_res) {
                catalyst::logger.log(LogLevel::ERROR, "on_build_failure hook failed: {}", hook_res.error());
                return std::unexpected(
                    res.error() + "\nAdditionally, the on_build_failure hook failed with error: " + hook_res.error());
            }
            return std::unexpected(res.error());
        }
    }

    // TODO: check if all deps exist or we've been asked to refetch them
    if (!fs::exists(build_dir / "catalyst-libs") || parse_args.force_refetch || dep_missing(config)) {
        if (parse_args.force_refetch) {
            catalyst::logger.log(LogLevel::INFO, "Forcing refetch of dependencies.");
            fs::remove_all(fs::path{build_dir / "catalyst-libs"}); // cleanup
        }
        catalyst::logger.log(LogLevel::INFO, "Fetching dependencies.");
        if (auto res = catalyst::fetch::action({parse_args.profiles}); !res) {
            catalyst::logger.log(LogLevel::ERROR, "Failed to fetch dependencies: {}", res.error());
            if (auto hook_res = hooks::on_build_failure(config); !hook_res) {
                catalyst::logger.log(LogLevel::ERROR, "on_build_failure hook failed: {}", hook_res.error());
                return std::unexpected(
                    res.error() + "\nAdditionally, the on_build_failure hook failed with error: " + hook_res.error());
            }
            return std::unexpected(res.error());
        }
    }

    catalyst::logger.log(LogLevel::INFO, "Building project.");
    if (std::system(std::format("ninja -C {}", build_dir.string()).c_str()) != 0) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to build project.");
        if (auto hook_res = hooks::on_build_failure(config); !hook_res) {
            catalyst::logger.log(LogLevel::ERROR, "on_build_failure hook failed: {}", hook_res.error());
            return std::unexpected(
                "Failed to build project.\nAdditionally, the on_build_failure hook failed with error: " +
                hook_res.error());
        }
        return std::unexpected("Failed to build project");
    }

    catalyst::logger.log(LogLevel::INFO, "Generating compile commands.");
    // effectively: call "ninja -C build_dir -t compdb > build_dir/compile_commands.json"
    if (auto res = generate_compile_commands(build_dir); !res) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to generate compile commands: {}", res.error());
        if (auto hook_res = hooks::on_build_failure(config); !hook_res) {
            catalyst::logger.log(LogLevel::ERROR, "on_build_failure hook failed: {}", hook_res.error());
            return std::unexpected(res.error() +
                                   "\nAdditionally, the on_build_failure hook failed with error: " + hook_res.error());
        }
        return res;
    }

    catalyst::logger.log(LogLevel::INFO, "Running post-build hooks.");
    if (auto res = hooks::post_build(config); !res) {
        catalyst::logger.log(LogLevel::ERROR, "Post-build hook failed: {}", res.error());
        if (auto hook_res = hooks::on_build_failure(config); !hook_res) {
            catalyst::logger.log(LogLevel::ERROR, "on_build_failure hook failed: {}", hook_res.error());
            return std::unexpected(res.error() +
                                   "\nAdditionally, the on_build_failure hook failed with error: " + hook_res.error());
        }
        return res;
    }
    catalyst::logger.log(LogLevel::INFO, "Build subcommand finished successfully.");
    return {};
}

} // namespace catalyst::build
