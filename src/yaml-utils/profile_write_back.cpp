#include <catalyst/yaml-utils/profile_write_back.hpp>
#include <expected>
#include <filesystem>
#include <fstream>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

namespace catalyst::YAML_UTILS {
std::expected<void, std::string> profile_write_back(const std::string &profile_name, YAML::Node &&node) {
    namespace fs = std::filesystem;
    fs::path profile_path;
    if (profile_name == "common")
        profile_path = "catalyst.yaml";
    else
        profile_path = std::format("catalyst_{}.yaml", profile_name);

    if (!fs::exists(profile_path)) {
        // log that we're creating a new file
    }

    std::ofstream profile_file{profile_path};
    YAML::Emitter emmiter;
    emmiter << node;
    profile_file << emmiter.c_str() << std::endl;
    return {};
}
} // namespace catalyst::YAML_UTILS
