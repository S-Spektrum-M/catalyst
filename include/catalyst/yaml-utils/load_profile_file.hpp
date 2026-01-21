#pragma once
#include <expected>
#include <string>
#include <yaml-cpp/yaml.h>
#include <filesystem>

namespace catalyst::YAML_UTILS {
std::expected<YAML::Node, std::string> load_profile_file(const std::string &profile, const std::filesystem::path& root_dir = std::filesystem::current_path());
}
