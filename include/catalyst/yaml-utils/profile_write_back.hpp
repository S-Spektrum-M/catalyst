#include <expected>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

namespace catalyst::YAML_UTILS {
std::expected<void, std::string> profileWriteBack(const std::string &profile_name, const YAML::Node &node);
}
