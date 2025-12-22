#include "catalyst/log-utils/log.hpp"
#include "catalyst/process_exec.h"

#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
#include <iostream>
#include <print>
#include <string>
#include <yaml-cpp/yaml.h>

namespace catalyst::fetch {

namespace fs = std::filesystem;

std::expected<YAML::Node, std::string> fetch_profile(const std::string &profile_name) {
    catalyst::logger.log(LogLevel::DEBUG, "Fetching profile: {}", profile_name);
    fs::path profile_path;
    if (profile_name == "common")
        profile_path = "catalyst.yaml";
    else
        profile_path = std::format("catalyst_{}.yaml", profile_name);
    if (!fs::exists(profile_path)) {
        catalyst::logger.log(LogLevel::ERROR, "Configuration file not found for profile: {}", profile_name);
        return std::unexpected(
            std::format("configuration: {} for profile: {} does not exist", profile_path.string(), profile_name));
    }
    return YAML::LoadFile(profile_path);
}

std::expected<void, std::string> fetch_vcpkg(const std::string &name) {
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
    if (catalyst::R_process_exec({vcpkg_exe.string(), "install", name}).value().get() != 0) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to fetch dependency: {}", name);
        return std::unexpected(std::format("Failed to fetch dependency: {}", name));
    }
    return {};
}

std::expected<void, std::string>
fetch_git(std::string build_dir, std::string name, std::string source, std::string version) {
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
    if (catalyst::R_process_exec(std::move(args)).value().get() != 0) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to fetch dependency: {}", name);
        return std::unexpected(std::format("Failed to fetch dependency: {}", name));
    }
    return {};
}

std::expected<void, std::string> fetch_system(const std::string &name) {
    // assuming installed on system
    catalyst::logger.log(LogLevel::DEBUG, "Skipping fetch for system dependency: {}", name);
    return {};
}

std::expected<void, std::string> fetch_local() {
    return {};
}
} // namespace catalyst::fetch
