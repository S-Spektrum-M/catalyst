#include "catalyst/hooks.hpp"
#include "catalyst/subcommands/generate.hpp"
#include "catalyst/subcommands/run.hpp"
#include <cctype>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

namespace catalyst::run {
std::string command_str(fs::path executable_name, const std::vector<std::string> &params);

std::expected<void, std::string> action(const parse_t &args) {
    std::vector<std::string> profiles;
    if (args.profile != "common") {
        profiles.push_back("common");
    }
    profiles.push_back(args.profile);

    YAML::Node profile_comp;
    if (auto res = generate::profile_composition(profiles); !res) {
        return std::unexpected(res.error());
    } else {
        profile_comp = res.value();
    }

    if (auto res = hooks::pre_run(profile_comp); !res) {
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
        return std::unexpected("profile does not build a binary target");
    }

    if (!profile_comp["manifest"]["dirs"]["build"]) {
        return std::unexpected("build directory is not defined");
    } else {
        build_dir = profile_comp["manifest"]["dirs"]["build"].as<std::string>();
    }

    if (profile_comp["manifest"]["provides"] && profile_comp["manifest"]["provides"].as<std::string>() != "") {
        exe = profile_comp["manifest"]["provides"].as<std::string>();
    } else if (profile_comp["manifest"]["name"] && profile_comp["manifest"]["name"].as<std::string>() != "") {
        exe = profile_comp["manifest"]["name"].as<std::string>();
    } else {
        return std::unexpected("unable to figure out executable name. "
                               "manifest.name and manifest.provides are undefined");
    }

    fs::path exe_path = fs::absolute(fs::path(std::format("{}/{}", build_dir, exe)));
    std::string command = command_str(exe_path, args.params);
    if (int res = std::system(command.c_str()); res) {
        return std::unexpected(std::format("exitied with code: {}", res));
    }

    if (auto res = hooks::post_run(profile_comp); !res) {
        return res;
    }

    return {};
}

std::string command_str(fs::path executable, const std::vector<std::string> &params) {
    std::string command = executable;
    for (const auto &param : params) {
        command += " " + param;
    }
    return command;
}
} // namespace catalyst::run
