#include "catalyst/log-utils/log.hpp"
#include "catalyst/process_exec.h"

#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
#include <iostream>
#include <print>
#include <sstream>
#include <string>
#include <unordered_map>
#include <yaml-cpp/yaml.h>

namespace catalyst::fetch {

namespace fs = std::filesystem;

std::expected<void, std::string> fetchVcpkg(const std::string &name) {
    catalyst::logger.log(LogLevel::DEBUG, "Fetching vcpkg dependency: {}", name);
    char *vcpkg_root_env = std::getenv("VCPKG_ROOT");
    if (vcpkg_root_env == nullptr) {
        catalyst::logger.log(LogLevel::ERROR, "VCPKG_ROOT environment variable not set.");
        return std::unexpected(
            "VCPKG_ROOT environment variable not set. Please set it to your vcpkg installation directory.");
    }
    fs::path vcpkg_root(vcpkg_root_env);
    fs::path vcpkg_exe = vcpkg_root / "vcpkg";
#if defined(_WIN32)
    vcpkg_exe.replace_extension(".exe");
#endif
    std::string command = std::format("\"{}\" install {}", vcpkg_exe.string(), name);
    catalyst::logger.log(LogLevel::DEBUG, "Executing command: {}", command);
    catalyst::logger.log(LogLevel::DEBUG, "Fetching: {} from vcpkg", name);
    if (catalyst::process_exec({vcpkg_exe.string(), "install", name}).value().get() != 0) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to fetch dependency: {}", name);
        return std::unexpected(std::format("Failed to fetch dependency: {}", name));
    }
    return {};
}

std::expected<void, std::string>
fetchGit(std::string build_dir, std::string name, std::string source, std::string version) {
    catalyst::logger.log(LogLevel::DEBUG, "Fetching git dependency: {}@{} from {}", name, version, source);
    fs::path dep_path = fs::path(build_dir) / "catalyst-libs" / name;
    std::println(std::cout, "Fetching: {}@{} from {}", name, version, source);
    std::string command;
    std::vector<std::string> args = {"git", "clone", "--depth", "1"};
    if (version == "latest") {
        command = std::format("git clone --depth 1 {} {}", source, dep_path.string());
        args.push_back(source);
        args.push_back(dep_path.string());
    } else {
        command = std::format("git clone --depth 1 --branch {} {} {}", version, source, dep_path.string());
        args.push_back("--branch");
        args.push_back(version);
        args.push_back(source);
        args.push_back(dep_path.string());
    }
    catalyst::logger.log(LogLevel::DEBUG, "Executing command: {}", command);
    if (catalyst::process_exec(std::move(args)).value().get() != 0) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to fetch dependency: {}", name);
        return std::unexpected(std::format("Failed to fetch dependency: {}", name));
    }
    return {};
}

std::expected<void, std::string> fetchSystem(const std::string &name) {
    // assuming installed on system
    catalyst::logger.log(LogLevel::DEBUG, "Skipping fetch for system dependency: {}", name);
    return {};
}

std::expected<void, std::string>
fetchLocal(const std::string &name, const std::string &path, const std::vector<std::string> &profiles) {
    fs::path local_path = fs::absolute(path);
    std::string visited_env = std::getenv("CATALYST_VISITED") ? std::getenv("CATALYST_VISITED") : "";

    // Cycle detection
    std::stringstream ss(visited_env);
    std::string segment;
    while (std::getline(ss, segment, ':')) {
        if (!segment.empty() && fs::equivalent(fs::path(segment), local_path)) {
            return std::unexpected(std::format("Dependency cycle detected involving {}", local_path.string()));
        }
    }

    std::string new_visited = visited_env.empty() ? local_path.string() : visited_env + ":" + local_path.string();

    catalyst::logger.log(LogLevel::DEBUG, "Recursively building local dependency: {} at {}", name, local_path.string());
    std::println(std::cout, "Building local dependency: {} at {}", name, local_path.string());

    std::vector<std::string> args = {"catalyst", "build"};
    if (profiles.size() != 0) {
        args.push_back("--profiles");
        for (const auto &p : profiles) {
            args.push_back(p);
        }
    }

    std::unordered_map<std::string, std::string> env_map;
    env_map["CATALYST_VISITED"] = new_visited;

    auto res = catalyst::process_exec(std::move(args), local_path.string(), env_map);
    if (!res) {
        return std::unexpected(res.error());
    }

    if (res.value().get() != 0) {
        return std::unexpected(std::format("Failed to build local dependency: {}", name));
    }

    return {};
}
} // namespace catalyst::fetch
