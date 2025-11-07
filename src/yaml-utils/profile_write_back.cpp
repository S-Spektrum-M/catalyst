#include "catalyst/log-utils/log.hpp"
#include <catalyst/yaml-utils/profile_write_back.hpp>
#include <expected>
#include <filesystem>
#include <fstream>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

namespace catalyst::YAML_UTILS {
std::expected<void, std::string> profile_write_back(const std::string &profile_name, YAML::Node &&node) {
    catalyst::logger.log(LogLevel::DEBUG, "Writing profile to file: {}", profile_name);
    namespace fs = std::filesystem;
    fs::path profile_path;
    if (profile_name == "common")
        profile_path = "catalyst.yaml";
    else
        profile_path = std::format("catalyst_{}.yaml", profile_name);

    catalyst::logger.log(LogLevel::DEBUG, "Profile path: {}", profile_path.string());
    if (!fs::exists(profile_path)) {
        catalyst::logger.log(LogLevel::DEBUG, "Creating new profile file: {}", profile_path.string());
    }

    std::ofstream profile_file{profile_path};
    YAML::Emitter emmiter;
    emmiter << node;
    profile_file << emmiter.c_str() << std::endl;
    catalyst::logger.log(LogLevel::DEBUG, "Profile file written successfully.");
    return {};
}
} // namespace catalyst::YAML_UTILS
