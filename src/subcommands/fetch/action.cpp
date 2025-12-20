#include "catalyst/hooks.hpp"
#include "catalyst/log-utils/log.hpp"
#include "catalyst/subcommands/fetch.hpp"
#include "catalyst/subcommands/generate.hpp"

#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
#include <iostream>
#include <print>
#include <string>
#include <yaml-cpp/yaml.h>

namespace catalyst::fetch {
namespace fs = std::filesystem;

std::expected<YAML::Node, std::string> fetch_profile(const std::string &profile_name);
std::expected<void, std::string> fetch_vcpkg(const std::string &name);
std::expected<void, std::string>
fetch_local(std::string build_dir, std::string name, std::string source, std::string version);
std::expected<void, std::string>
fetch_git(std::string build_dir, std::string name, std::string source, std::string version);
std::expected<void, std::string> fetch_system(const std::string &name);

std::expected<void, std::string> action(const parse_t &parse_args) {
    catalyst::logger.log(LogLevel::DEBUG, "Fetch subcommand invoked.");
    catalyst::logger.log(LogLevel::DEBUG, "Composing profiles.");
    YAML_UTILS::Configuration config{parse_args.profiles};

    catalyst::logger.log(LogLevel::DEBUG, "Running pre-fetch hooks.");
    if (auto res = hooks::pre_fetch(config); !res) {
        catalyst::logger.log(LogLevel::ERROR, "Pre-fetch hook failed: {}", res.error());
        return res;
    }

    std::string build_dir = config.get_string("manifest.dirs.build").value_or("build");
    if (auto deps = config.get_root()["dependencies"]; deps && deps.IsSequence()) {
        for (auto dep : deps) {
            if (!dep["name"] || !dep["source"]) {
                catalyst::logger.log(LogLevel::ERROR, "Improperly configured dependency.");
                return std::unexpected("Improperly configured dependency.");
            }
            std::string name = dep["name"].as<std::string>();
            std::string source = dep["source"].as<std::string>();
            catalyst::logger.log(LogLevel::DEBUG, "Fetching dependency '{}' from '{}'", name, source);
            if (source == "vcpkg") {
                if (!dep["version"]) {
                    return std::unexpected(std::format("vcpkg dependency '{}' is missing version.", name));
                }
                if (auto res = fetch_vcpkg(name); !res)
                    return std::unexpected(res.error());
            } else if (source == "system") {
                if (auto res = fetch_system(name); !res)
                    return std::unexpected(res.error());
            } else if (source == "local") {
                std::println(std::cout, "Skipping fetch for local dependency: {}", name);
            } else {
                fs::path dep_path = fs::path(build_dir) / "catalyst-libs" / name;
                if (fs::exists(dep_path)) {
                    std::println(std::cout, "Skipping fetch for existing git dependency: {}", name);
                } else {
                    if (!dep["version"] || !dep["version"].IsScalar()) {
                        return std::unexpected(std::format("git dependency '{}' is missing version.", name));
                    }
                    std::string version = dep["version"].as<std::string>();
                    if (auto res = fetch_git(build_dir, name, source, version); !res)
                        return std::unexpected(res.error());
                }
            }
        }
    }

    catalyst::logger.log(LogLevel::DEBUG, "Running post-fetch hooks.");
    if (auto res = hooks::post_fetch(config); !res) {
        catalyst::logger.log(LogLevel::ERROR, "Post-fetch hook failed: {}", res.error());
        return res;
    }

    catalyst::logger.log(LogLevel::DEBUG, "Fetch subcommand finished successfully.");
    return {};
}
} // namespace catalyst::fetch
