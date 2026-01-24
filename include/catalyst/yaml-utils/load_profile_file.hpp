#pragma once
#include <expected>
#include <filesystem>
#include <string>
#include <yaml-cpp/yaml.h>

namespace catalyst::yaml_utils {
std::expected<YAML::Node, std::string>
loadProfileFile(const std::string &profile, const std::filesystem::path &root_dir = std::filesystem::current_path());
}
