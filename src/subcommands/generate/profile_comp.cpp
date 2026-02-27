#include <sys/wait.h>

#include <exception>
#include <expected>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

#include "catalyst/log_utils/log.hpp"
#include "catalyst/subcommands/generate.hpp"
#include "catalyst/yaml_utils/configuration.hpp"

#include "yaml-cpp/node/node.h"

namespace catalyst::generate {
// NOTE: eventually get rid of all calls to profile_composition
std::expected<YAML::Node, std::string> profileComposition(const std::vector<std::string> &p) {
    catalyst::logger.log(LogLevel::DEBUG, "Composing profiles.");
    catalyst::logger.log(LogLevel::DEBUG, "Profile composition finished.");
    try {
        return YAML::Clone(yaml_utils::Configuration{p}.get_root());
    } catch (std::exception &err) {
        return std::unexpected(err.what());
    }
}
} // namespace catalyst::generate
