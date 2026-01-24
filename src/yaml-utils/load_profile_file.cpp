#include "catalyst/log-utils/log.hpp"

#include <catalyst/yaml-utils/load_profile_file.hpp>
#include <expected>
#include <filesystem>
#include <format>
#include <string>
#include <yaml-cpp/exceptions.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

namespace catalyst::yaml_utils {
std::expected<YAML::Node, std::string> loadProfileFile(const std::string &profile,
                                                       const std::filesystem::path &root_dir) {
    catalyst::logger.log(LogLevel::DEBUG, "Loading profile file: {} from {}", profile, root_dir.string());
    namespace fs = std::filesystem;
    fs::path profile_path = root_dir;
    if (profile == "common")
        profile_path /= "catalyst.yaml";
    else
        profile_path /= std::format("catalyst_{}.yaml", profile);

    catalyst::logger.log(LogLevel::DEBUG, "Profile path: {}", profile_path.string());
    if (!fs::exists(profile_path)) {
        catalyst::logger.log(LogLevel::ERROR, "Profile file not found: {}", profile_path.string());
        return std::unexpected(std::format("profile file: {} not found", profile_path.string()));
    }

    try {
        YAML::Node ret = YAML::LoadFile(profile_path);
        catalyst::logger.log(LogLevel::DEBUG, "Profile file loaded successfully.");
        return ret;
    } catch (YAML::Exception &err) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to parse YAML file: {}", err.what());
        return std::unexpected(err.what());
    }
}
} // namespace catalyst::yaml_utils
