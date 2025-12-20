#include "catalyst/log-utils/log.hpp"
#include "catalyst/subcommands/generate.hpp"
#include "yaml-cpp/node/node.h"

#include <expected>
#include <filesystem>
#include <format>
#include <string>

namespace catalyst::generate {
namespace fs = std::filesystem;

std::expected<find_res, std::string> find_git(const std::string &build_dir, const YAML::Node &dep) {
    std::string dep_name = dep["name"].as<std::string>();
    catalyst::logger.log(LogLevel::DEBUG, "Resolving git dependency: {}", dep_name);

    auto project_dir = fs::current_path();
    fs::path dep_path = fs::path(build_dir) / "catalyst-libs" / dep_name;

    catalyst::logger.log(LogLevel::DEBUG, "Changing directory to: {}", dep_path.string());
    try {
        fs::current_path(dep_path);
    } catch (const fs::filesystem_error &e) {
        return std::unexpected(
            std::format("Git dependency path not found: {}. Did you run 'catalyst fetch'?", dep_path.string()));
    }

    std::vector<std::string> profiles{};
    if (dep["profiles"] && dep["profiles"].IsSequence())
        profiles = dep["profiles"].as<std::vector<std::string>>();

    catalyst::logger.log(LogLevel::DEBUG, "Composing profiles for git dependency.");
    auto pc = catalyst::generate::profile_composition(profiles);

    fs::current_path(project_dir);
    catalyst::logger.log(LogLevel::DEBUG, "Changing directory back to: {}", project_dir.string());

    if (!pc) {
        return std::unexpected(pc.error());
    }
    YAML::Node profile = pc.value();

    std::string include_path_flags;
    if (auto includes = profile["manifest"]["dirs"]["include"]; includes && includes.IsSequence()) {
        for (const auto &dir : includes.as<std::vector<std::string>>()) {
            auto abs_include_path = fs::absolute(dep_path / dir);
            catalyst::logger.log(LogLevel::DEBUG, "Adding include path: {}", abs_include_path.string());
            include_path_flags += std::format(" -I{}", abs_include_path.string());
        }
    }

    std::string library_path_flags;
    if (auto build_dir_node = profile["manifest"]["dirs"]["build"]) {
        auto dep_build_dir = build_dir_node.as<std::string>();
        auto lib_path = fs::absolute(dep_path / dep_build_dir);
        catalyst::logger.log(LogLevel::DEBUG, "Adding library path: {}", lib_path.string());
        library_path_flags += std::format(" -L{}", lib_path.string());
    }

    std::string libs_flags;
    if (auto dep_name_node = profile["manifest"]["name"]) {
        auto lib_name = dep_name_node.as<std::string>();
        catalyst::logger.log(LogLevel::DEBUG, "Adding library: {}", lib_name);
        libs_flags += std::format(" -l{}", lib_name);
    }

    return find_res{.lib_path = library_path_flags, .inc_path = include_path_flags, .libs = libs_flags};
}
} // namespace catalyst::generate