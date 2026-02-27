#pragma once
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "yaml-cpp/yaml.h"

namespace catalyst::yaml_utils {
class Configuration {
public:
    Configuration() = default;
    explicit Configuration(const std::vector<std::string> &profiles,
                           const std::filesystem::path &root_dir = std::filesystem::current_path());

    bool has(const std::string &key) const;

    std::optional<std::string> get_string(const std::string &key) const;
    std::optional<int> get_int(const std::string &key) const;
    std::optional<bool> get_bool(const std::string &key) const;
    std::optional<std::vector<std::string>> get_string_vector(const std::string &key) const;

    const YAML::Node &get_root() const & {
        return root;
    }

private:
    YAML::Node root;
};
} // namespace catalyst::yaml_utils
