#include "catalyst/log-utils/log.hpp"
#include "catalyst/subcommands/generate.hpp"
#include "yaml-cpp/node/node.h"
#include <expected>
#include <filesystem>
#include <format>
#include <string>

using std::string;

namespace catalyst::generate {
std::expected<find_res, std::string> find_dep(const std::string &build_dir, const YAML::Node &dep) {
    if (!dep["source"] || !dep["source"].IsScalar())
        return std::unexpected(std::format("dependency: {} does not define field src", dep["name"].as<std::string>()));
    std::string source_type = dep["source"].as<std::string>();
    if (source_type == "local") {
        return find_local(dep);
    } else if (source_type == "system") {
        return find_system(dep);
    } else if (source_type == "vcpkg") {
        return find_vcpkg(dep);
    } else if (source_type == "git") {
        return find_git(build_dir, dep);
    } else
        return std::unexpected(std::format("Unkown source_type: {}", source_type));
}
} // namespace catalyst::generate
