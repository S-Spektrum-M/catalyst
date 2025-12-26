#include "catalyst/hooks.hpp"
#include "catalyst/log-utils/log.hpp"
#include "catalyst/process_exec.h"
#include "catalyst/subcommands/generate.hpp"
#include "catalyst/subcommands/run.hpp"

#include <cctype>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
#include <sstream>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

namespace catalyst::run {
std::string command_str(fs::path executable_name, const std::vector<std::string> &params);

std::expected<void, std::string> action(const parse_t &args) {
    catalyst::logger.log(LogLevel::DEBUG, "Run subcommand invoked.");
    std::vector<std::string> profiles;
    if (args.profile != "common") {
        profiles.push_back("common");
    }
    profiles.push_back(args.profile);

    catalyst::logger.log(LogLevel::DEBUG, "Composing profiles.");
    YAML::Node profile_comp;
    if (std::expected<YAML::Node, std::string> res = generate::profile_composition(profiles); !res) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to compose profiles: {}", res.error());
        return std::unexpected(res.error());
    } else {
        profile_comp = res.value();
    }

    catalyst::logger.log(LogLevel::DEBUG, "Running pre-run hooks.");
    if (std::expected<void, std::string> res = hooks::pre_run(profile_comp); !res) {
        catalyst::logger.log(LogLevel::ERROR, "Pre-run hook failed: {}", res.error());
        return res;
    }

    std::string exe, build_dir;
    auto str_to_lower = [](std::string &input) -> void {
        auto lower = [](const char c) -> char { return std::tolower(c); };
        std::transform(input.begin(), input.end(), input.begin(), lower);
        return;
    };
    std::string target_type = profile_comp["manifest"]["type"].Scalar();
    str_to_lower(target_type);

    if (!profile_comp["manifest"]["type"].IsDefined() || target_type != "binary") {
        if (!profile_comp["manifest"]["type"].IsDefined()) {
            catalyst::logger.log(LogLevel::ERROR, "Profile: {} does not define field: 'manifest.type'.", args.profile);
            return std::unexpected(std::format("Profile: {} does not define field: 'manifest.type'.", args.profile));
        } else {
            catalyst::logger.log(LogLevel::ERROR,
                                 "Profile: {} defines 'manifest.type' = {}. Expected 'manifest.type' = BINARY",
                                 args.profile,
                                 target_type);
            return std::unexpected(
                std::format("Profile: {} defines 'manifest.type' = {}. Expected 'manifest.type' = BINARY",
                            args.profile,
                            target_type));
        }
        return std::unexpected(std::format("Profile: {} does not build a binary target.", args.profile));
    }

    if (!profile_comp["manifest"]["dirs"]["build"]) {
        catalyst::logger.log(LogLevel::ERROR, "Build directory is not defined in profile: {}.", args.profile);
        return std::unexpected(std::format("Build directory is not defined in profile: {}.", args.profile));
    } else {
        build_dir = profile_comp["manifest"]["dirs"]["build"].as<std::string>();
    }

    if (profile_comp["manifest"]["provides"] && profile_comp["manifest"]["provides"].as<std::string>() != "") {
        exe = profile_comp["manifest"]["provides"].as<std::string>();
    } else if (profile_comp["manifest"]["name"] && profile_comp["manifest"]["name"].as<std::string>() != "") {
        exe = profile_comp["manifest"]["name"].as<std::string>();
    } else {
        catalyst::logger.log(LogLevel::ERROR, "Unable to determine executable name.");
        return std::unexpected("Unable to figure out executable name."
                               "manifest.name and manifest.provides are undefined");
    }

    fs::path exe_path = fs::absolute(fs::path(std::format("{}/{}", build_dir, exe)));
    std::string command = command_str(exe_path, args.params);
    if (std::expected<std::string, std::string> res = catalyst::generate::lib_path(profile_comp); !res) {
        return std::unexpected("Failed to generate LD_LIBRARY_PATH");
    } else {
#if defined(_WIN32)
        _putenv_s("PATH", res.value().c_str());
#elif defined(__APPLE__)
        setenv("DYLD_LIBRARY_PATH", res.value().c_str(), 1);
#else
        setenv("LD_LIBRARY_PATH", res.value().c_str(), 1);
#endif
    }
    catalyst::logger.log(LogLevel::DEBUG, "Executing command: {}", command);

    std::vector<std::string> exec_args;
    exec_args.push_back(exe_path.string());
    exec_args.insert(exec_args.end(), args.params.begin(), args.params.end());

    if (int res = catalyst::process_exec(std::move(exec_args)).value().get(); res) {
        catalyst::logger.log(LogLevel::ERROR, "Target exited with code: {}", res);
        return std::unexpected(
            std::format("Target executable: {} exited with failure code: {}", exe_path.string(), res));
    }

    catalyst::logger.log(LogLevel::DEBUG, "Running post-run hooks.");
    if (std::expected<void, std::string> res = hooks::post_run(profile_comp); !res) {
        catalyst::logger.log(LogLevel::ERROR, "Post-run hook failed: {}", res.error());
        return res;
    }

    catalyst::logger.log(LogLevel::DEBUG, "Run subcommand finished successfully.");
    return {};
}

std::string command_str(fs::path executable, const std::vector<std::string> &params) {
    catalyst::logger.log(LogLevel::DEBUG, "Constructing command string.");
    std::string command = executable;
    for (const auto &param : params) {
        command += " " + param;
    }
    return command;
}
} // namespace catalyst::run
