#include "catalyst/log-utils/log.hpp"
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
    catalyst::logger.log(LogLevel::INFO, "Fetching profile: {}", profile_name);
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
    catalyst::logger.log(LogLevel::INFO, "Fetching vcpkg dependency: {}", name);
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
    catalyst::logger.log(LogLevel::INFO, "Executing command: {}", command);
    std::println(std::cout, "Fetching: {} from vcpkg", name);
    if (std::system(command.c_str()) != 0) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to fetch dependency: {}", name);
        return std::unexpected(std::format("Failed to fetch dependency: {}", name));
    }
    return {};
}

std::expected<void, std::string> fetch_git(std::string build_dir, std::string name, std::string source,
                                           std::string version) {
    catalyst::logger.log(LogLevel::INFO, "Fetching git dependency: {}@{} from {}", name, version, source);
    fs::path dep_path = fs::path(build_dir) / "catalyst-libs" / name;
    std::println(std::cout, "Fetching: {}@{} from {}", name, version, source);
    std::string command;
    if (version == "latest") {
        command = std::format("git clone --depth 1 {} {}", source, dep_path.string());
    } else {
        command = std::format("git clone --depth 1 --branch {} {} {}", version, source, dep_path.string());
    }
    catalyst::logger.log(LogLevel::INFO, "Executing command: {}", command);
    if (std::system(command.c_str()) != 0) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to fetch dependency: {}", name);
        return std::unexpected(std::format("Failed to fetch dependency: {}", name));
    }
    return {};
}

std::expected<void, std::string> fetch_system(const std::string &name) {
    // assuming installed on system
    catalyst::logger.log(LogLevel::INFO, "Skipping fetch for system dependency: {}", name);
    return {};
}

std::expected<void, std::string> fetch_local() { return {}; }
} // namespace catalyst::fetch
