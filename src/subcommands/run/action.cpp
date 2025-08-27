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
std::expected<fs::path, std::string> executable_name(const std::string &profile);
std::string command_str(fs::path executable_name, const std::vector<std::string> &params);

std::expected<void, std::string> action(const parse_t &args) {
    // TODO: better error handling
    std::string exe_name;
    if (auto res = executable_name(args.profile); !res) {
        return std::unexpected(res.error());
    } else {
        exe_name = res.value();
    }

    std::string command = command_str(exe_name, args.params);
    if (int res = std::system(command.c_str()); res) {
        return std::unexpected(std::format("exitied with code: {}", res));
    }
    return {};
}

std::expected<fs::path, std::string> executable_name(const std::string &profile) {
    fs::path profile_file;
    if (profile == "common") {
        profile_file = "catalyst.yaml";
    } else {
        profile_file = std::format("catalyst_{}.yaml", profile);
    }

    if (!fs::exists(profile_file)) {
        return std::unexpected(std::format("configuration for profile: {} not found", profile));
    }

    YAML::Node profile_node = YAML::LoadFile(profile_file);
    std::string exe, build_dir;
    auto str_to_lower = [](std::string &input) -> void {
        auto lower = [](const char c) -> char { return std::tolower(c); };
        std::transform(input.begin(), input.end(), input.begin(), lower);
        return;
    };
    std::string target_type = profile_node["manifest"]["type"].Scalar();
    str_to_lower(target_type);

    if (!profile_node["manifest"]["type"].IsDefined() || target_type != "binary") {
        std::cerr << target_type << std::endl;
        return std::unexpected("profile does not build a binary target");
    }

    if (!profile_node["manifest"]["dirs"]["build"]) {
        return std::unexpected("build directory is not defined");
    } else {
        build_dir = profile_node["manifest"]["dirs"]["build"].as<std::string>();
    }

    if (profile_node["manifest"]["provides"] && profile_node["manifest"]["provides"].as<std::string>() != "") {
        exe = profile_node["manifest"]["provides"].as<std::string>();
    } else if (profile_node["manifest"]["name"] && profile_node["manifest"]["name"].as<std::string>() != "") {
        exe = profile_node["manifest"]["name"].as<std::string>();
    } else {
        return std::unexpected("unable to figure out executable name. "
                               "manifest.name and manifest.provides are undefined");
    }

    return fs::absolute(fs::path(std::format("{}/{}", build_dir, exe)));
}

std::string command_str(fs::path executable, const std::vector<std::string> &params) {
    std::string command = executable;
    for (const auto &param : params) {
        command += " " + param;
    }
    return command;
}
} // namespace catalyst::run
