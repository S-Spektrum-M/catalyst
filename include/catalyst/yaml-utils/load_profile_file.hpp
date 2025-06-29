#pragma once
#include <expected>
#include <string>
#include <yaml-cpp/yaml.h>

namespace catalyst::YAML_UTILS {
    std::expected<YAML::Node, std::string> load_profile_file(const std::string &profile);
}
