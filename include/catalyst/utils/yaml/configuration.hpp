#pragma once
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "yaml-cpp/yaml.h"

namespace catalyst::utils::yaml {
class Configuration {
public:
    Configuration() = default;
    explicit Configuration(const std::vector<std::string> &profiles,
                           const std::filesystem::path &root_dir = std::filesystem::current_path());

    bool has(const std::string &key) const;

    std::optional<std::string> getString(const std::string &key) const;
    std::optional<int> getInt(const std::string &key) const;
    std::optional<bool> getBool(const std::string &key) const;
    std::optional<std::vector<std::string>> getStringVector(const std::string &key) const;

    const YAML::Node &getRoot() const & {
        return root;
    }

private:
    YAML::Node root;
};
} // namespace catalyst::utils::yaml
