#include <expected>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

namespace catalyst::YAML_UTILS {
std::expected<void, std::string> profile_write_back(const std::string &profile_name, YAML::Node &&node);
}
