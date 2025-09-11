#pragma once
#include "yaml-cpp/yaml.h"
#include <optional>
#include <string>
#include <vector>

namespace catalyst::YAML_UTILS {
class Configuration {
  public:
    Configuration(const std::vector<std::string> &profiles);

    bool has(const std::string &key) const;

    std::optional<std::string> get_string(const std::string &key) const;
    std::optional<int> get_int(const std::string &key) const;
    std::optional<bool> get_bool(const std::string &key) const;
    std::optional<std::vector<std::string>> get_string_vector(const std::string &key) const;

  private:
    YAML::Node root;
};
} // namespace catalyst::YAML_UTILS
