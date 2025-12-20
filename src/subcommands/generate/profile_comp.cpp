#include "catalyst/log-utils/log.hpp"
#include "catalyst/subcommands/generate.hpp"
#include "catalyst/yaml-utils/Configuration.hpp"
#include "yaml-cpp/node/node.h"

#include <exception>
#include <expected>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <sys/wait.h>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace catalyst::generate {
// NOTE: eventually get rid of all calls to profile_composition
std::expected<YAML::Node, std::string> profile_composition(const std::vector<std::string> &p) {
    catalyst::logger.log(LogLevel::DEBUG, "Composing profiles.");
    catalyst::logger.log(LogLevel::DEBUG, "Profile composition finished.");
    try {
        return YAML::Clone(YAML_UTILS::Configuration{p}.get_root());
    } catch (std::exception &err) {
        return std::unexpected(err.what());
    }
}
} // namespace catalyst::generate
