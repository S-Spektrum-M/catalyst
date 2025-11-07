#include "catalyst/hooks.hpp"
#include "catalyst/log-utils/log.hpp"
#include "catalyst/subcommands/generate.hpp"
#include "catalyst/subcommands/run.hpp"
#include <cctype>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
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
    if (auto res = generate::profile_composition(profiles); !res) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to compose profiles: {}", res.error());
        return std::unexpected(res.error());
    } else {
        profile_comp = res.value();
    }

    catalyst::logger.log(LogLevel::DEBUG, "Running pre-run hooks.");
    if (auto res = hooks::pre_run(profile_comp); !res) {
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
        catalyst::logger.log(LogLevel::ERROR, "Profile does not build a binary target.");
        return std::unexpected("profile does not build a binary target");
    }

    if (!profile_comp["manifest"]["dirs"]["build"]) {
        catalyst::logger.log(LogLevel::ERROR, "Build directory is not defined.");
        return std::unexpected("build directory is not defined");
    } else {
        build_dir = profile_comp["manifest"]["dirs"]["build"].as<std::string>();
    }

    if (profile_comp["manifest"]["provides"] && profile_comp["manifest"]["provides"].as<std::string>() != "") {
        exe = profile_comp["manifest"]["provides"].as<std::string>();
    } else if (profile_comp["manifest"]["name"] && profile_comp["manifest"]["name"].as<std::string>() != "") {
        exe = profile_comp["manifest"]["name"].as<std::string>();
    } else {
        catalyst::logger.log(LogLevel::ERROR, "Unable to determine executable name.");
        return std::unexpected("unable to figure out executable name. "
                               "manifest.name and manifest.provides are undefined");
    }

    fs::path exe_path = fs::absolute(fs::path(std::format("{}/{}", build_dir, exe)));
    std::string command = command_str(exe_path, args.params);
    if (auto res = catalyst::generate::lib_path(profile_comp); !res) {
        return std::unexpected("failed to generate LD_LIBRARY_PATH");
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
    if (int res = std::system(command.c_str()); res) {
        catalyst::logger.log(LogLevel::ERROR, "Command exited with code: {}", res);
        return std::unexpected(std::format("exitied with code: {}", res));
    }

    catalyst::logger.log(LogLevel::DEBUG, "Running post-run hooks.");
    if (auto res = hooks::post_run(profile_comp); !res) {
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
