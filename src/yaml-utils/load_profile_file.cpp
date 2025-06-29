#include <catalyst/yaml-utils/load_profile_file.hpp>
#include <expected>
#include <filesystem>
#include <format>
#include <string>
#include <yaml-cpp/exceptions.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

namespace catalyst::YAML_UTILS {
std::expected<YAML::Node, std::string> load_profile_file(const std::string &profile) {
    namespace fs = std::filesystem;
    fs::path profile_path;
    if (profile == "common")
        profile_path = "catalyst.yaml";
    else
        profile_path = std::format("catalyst_{}.yaml", profile);

    if (!fs::exists(profile_path))
        return std::unexpected(std::format("profile file: {} not found", profile_path.string()));

    try {
        auto ret = YAML::LoadFile(profile_path);
        return ret;
    } catch (YAML::Exception &err) {
        return std::unexpected(err.what());
    }
}
} // namespace catalyst::YAML_UTILS
