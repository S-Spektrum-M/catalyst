#include "catalyst/log-utils/log.hpp"
#include "catalyst/subcommands/generate.hpp"
#include "yaml-cpp/node/node.h"
#include <expected>
#include <format>
#include <string>

namespace catalyst::generate {
namespace fs = std::filesystem;

std::expected<find_res, std::string> find_local(const YAML::Node &dep) {
    catalyst::logger.log(LogLevel::INFO, "Resolving local dependency: {}", dep["name"].as<std::string>());
    auto project_dir = fs::current_path(); // save cwd
    fs::path dep_path;
    if (dep["path"]) {
        dep_path = dep["path"].as<std::string>();
        catalyst::logger.log(LogLevel::INFO, "Changing directory to: {}", dep_path.string());
        fs::current_path(dep_path); // navigate to the dep_path
    } else {
        return std::unexpected(
            std::format("Local Dependency: {} does not define path.", dep["name"].as<std::string>()));
    }

    std::vector<std::string> profiles{}, features{};
    if (dep["profiles"] && dep["profiles"].IsSequence())
        profiles = dep["profiles"].as<std::vector<std::string>>();
    if (dep["using"] && dep["using"].IsSequence())
        features = dep["using"].as<std::vector<std::string>>();
    catalyst::logger.log(LogLevel::INFO, "Composing profiles for local dependency.");
    auto pc = catalyst::generate::profile_composition(profiles);

    fs::current_path(project_dir);
    if (!pc) {
        return std::unexpected(pc.error());
    }
    YAML::Node profile = pc.value();

    // DO NOT rebuild here. This should have been done in fetch::fetch_local or something.

    catalyst::logger.log(LogLevel::INFO, "Changing directory back to: {}", project_dir.string());

    // Add include directories
    std::string include_path;
    if (auto includes = profile["manifest"]["dirs"]["include"]; includes && includes.IsSequence()) {
        for (const auto &dir : includes.as<std::vector<std::string>>()) {
            auto curr = fs::absolute(dep_path / dir);
            catalyst::logger.log(LogLevel::INFO, "Adding include path: {}", curr.string());
            include_path += std::format(" -I{}", curr.string());
        }
    }

    // Add library directory
    std::string library_path;
    if (auto build_dir_node = profile["manifest"]["dirs"]["build"]) {
        auto build_dir = build_dir_node.as<std::string>();
        auto lib_path = fs::absolute(dep_path / build_dir);
        catalyst::logger.log(LogLevel::INFO, "Adding library path: {}", lib_path.string());
        library_path += std::format(" -L{}", lib_path.string());
    }

    // Add library
    std::string libs;
    if (auto dep_name_node = profile["manifest"]["name"]) {
        auto dep_name = dep_name_node.as<std::string>();
        catalyst::logger.log(LogLevel::INFO, "Adding library: {}", dep_name);
        libs += std::format(" -l{}", dep_name);
    }

    return find_res{.lib_path = library_path, .inc_path = include_path, .libs = libs};
}
} // namespace catalyst::generate
