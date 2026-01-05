#include "catalyst/hooks.hpp"
#include "catalyst/log-utils/log.hpp"
#include "catalyst/process_exec.h"
#include "catalyst/subcommands/build.hpp"
#include "catalyst/subcommands/fetch.hpp"
#include "catalyst/subcommands/generate.hpp"
#include "catalyst/yaml-utils/Configuration.hpp"

#include <algorithm>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <vector>
#include <yaml-cpp/node/node.h>

namespace catalyst::build {
namespace fs = std::filesystem;

bool dep_missing(const YAML_UTILS::Configuration &config) {
    catalyst::logger.log(LogLevel::DEBUG, "Checking for missing dependencies.");
    fs::path build_dir = config.get_string("manifest.dirs.build").value_or("build");
    if (!config.has("dependencies")) {
        catalyst::logger.log(LogLevel::DEBUG, "No dependencies declared, skipping check.");
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
    catalyst::logger.log(LogLevel::DEBUG, "All dependencies are present.");
    return false;
}

// FIXME: use better redirection scheme
std::expected<void, std::string> generate_compile_commands(const fs::path &build_dir, const std::string &generator) {
    if (generator != "ninja") { // TODO: wait for CBE to support compile_commands generation in 1.0
        catalyst::logger.log(LogLevel::DEBUG, "Skipping compile commands generation for generator: {}", generator);
        return {};
    }
    catalyst::logger.log(LogLevel::INFO, "Generating compile commands database.");
    fs::path real_compdb_path = build_dir / "compile_commands.json";
    std::string compdb_command =
        std::format("ninja -C {} -t compdb cc_compile cxx_compile > {}", build_dir.string(), real_compdb_path.string());
    catalyst::logger.log(LogLevel::DEBUG, "Executing command: {}", compdb_command);
    auto shell_cmd = [](const std::string &cmd) -> std::vector<std::string> {
#if defined(_WIN32)
        return {"cmd", "/c", cmd};
#else
        return {"/bin/sh", "-c", cmd};
#endif
    };
    if (int rtn = catalyst::process_exec(shell_cmd(compdb_command)).value().get(); rtn != 0) {
        return std::unexpected(std::format("Failed to generate compile_commands.json.\n"
                                           "Command: {} exited with error code: {}",
                                           compdb_command,
                                           rtn));
    }
    return {};
}

std::expected<void, std::string> action(const parse_t &parse_args) {
    catalyst::logger.log(LogLevel::DEBUG, "Build subcommand invoked.");
    catalyst::logger.log(LogLevel::DEBUG, "Composing profiles.");
    YAML_UTILS::Configuration config{parse_args.profiles};

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
    std::string generator = config.get_string("meta.generator").value_or("cbe");
    std::string build_filename = (generator == "ninja") ? "build.ninja" : "catalyst.build";

    if (!fs::exists(build_dir / build_filename) || parse_args.regen) {
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
            catalyst::logger.log(LogLevel::INFO, "Forcefully refetching dependencies.");
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
    std::vector<std::string> build_command;
    if (generator == "ninja") {
        build_command = {"ninja"};
    } else {
        build_command = {"cbe"};
    }

    if (int res = catalyst::process_exec(std::move(build_command), build_dir).value().get(); res != 0) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to build project.");
        if (auto hook_res = hooks::on_build_failure(config); !hook_res) {
            catalyst::logger.log(LogLevel::ERROR, "on_build_failure hook failed: {}", hook_res.error());
            return std::unexpected(
                "Failed to build project.\nAdditionally, the on_build_failure hook failed with error: " +
                hook_res.error());
        }
        return std::unexpected(std::format("Build process failed. {} exited with code: {}", build_command[0], res));
    }

    catalyst::logger.log(LogLevel::INFO, "Generating compile commands.");
    // effectively: call "ninja -C build_dir -t compdb > build_dir/compile_commands.json"
    if (auto res = generate_compile_commands(build_dir, generator); !res) {
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
