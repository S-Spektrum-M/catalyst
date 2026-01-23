#include "catalyst/hooks.hpp"
#include "catalyst/log-utils/log.hpp"
#include "catalyst/subcommands/fetch.hpp"

#include <expected>
#include <filesystem>
#include <format>
#include <iostream>
#include <print>
#include <string>
#include <yaml-cpp/yaml.h>

namespace catalyst::fetch {
namespace fs = std::filesystem;

std::expected<void, std::string> fetchVcpkg(const std::string &name);
std::expected<void, std::string>
fetchLocal(const std::string &name, const std::string &path, const std::vector<std::string> &profiles);
std::expected<void, std::string>
fetchGit(std::string build_dir, std::string name, std::string source, std::string version);
std::expected<void, std::string> fetchSystem(const std::string &name);

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
        for (int ii = 0; auto dep : deps) {
            if (!dep["name"]) {
                catalyst::logger.log(LogLevel::ERROR, "Dependency: {} does not define field: name", ii);
                return std::unexpected(std::format("Dependency: {} does not define field: name", ii));
            }
            if (!dep["source"]) {
                catalyst::logger.log(
                    LogLevel::ERROR, "Dependency: {} does not define field: source", dep["name"].as<std::string>());
                return std::unexpected(
                    std::format("Dependency: {} does not define field: source", dep["name"].as<std::string>()));
            }
            auto name = dep["name"].as<std::string>();
            auto source = dep["source"].as<std::string>();

            if (parse_args.workspace) {
                if (auto member = parse_args.workspace->find_package(name)) {
                    catalyst::logger.log(LogLevel::INFO, "Dependency '{}' found in workspace at '{}'. Linking...", name, member->path.string());
                    fs::path lib_path = fs::path(build_dir) / "catalyst-libs" / name;

                    try {
                        if (fs::exists(lib_path) || fs::is_symlink(lib_path)) {
                            if (fs::is_symlink(lib_path)) {
                                if (fs::read_symlink(lib_path) != member->path) {
                                    fs::remove(lib_path);
                                    fs::create_directory_symlink(member->path, lib_path);
                                }
                            } else {
                                fs::remove_all(lib_path);
                                fs::create_directory_symlink(member->path, lib_path);
                            }
                        } else {
                            fs::create_directories(lib_path.parent_path());
                            fs::create_directory_symlink(member->path, lib_path);
                        }
                    } catch (const std::exception& e) {
                         catalyst::logger.log(LogLevel::ERROR, "Failed to link workspace dependency: {}", e.what());
                         return std::unexpected(e.what());
                    }
                    continue;
                }
            }

            catalyst::logger.log(LogLevel::DEBUG, "Fetching dependency '{}' from '{}'", name, source);
            if (source == "vcpkg") {
                if (!dep["version"]) {
                    return std::unexpected(std::format("vcpkg dependency '{}' is missing version.", name));
                }
                if (!dep["triplet"]) {
                    return std::unexpected(std::format("vcpkg dependency '{}' is missing triplet.", name));
                }
                if (auto res = fetchVcpkg(name); !res)
                    return std::unexpected(res.error());
            } else if (source == "system") {
                if (auto res = fetchSystem(name); !res)
                    return std::unexpected(res.error());
            } else if (source == "local") {
                if (!dep["path"]) {
                    return std::unexpected(std::format("Local dependency '{}' is missing path.", name));
                }
                auto path = dep["path"].as<std::string>();
                std::vector<std::string> profiles_vec;
                if (dep["profiles"] && dep["profiles"].IsSequence()) {
                    profiles_vec = dep["profiles"].as<std::vector<std::string>>();
                }
                if (auto res = fetchLocal(name, path, profiles_vec); !res)
                    return std::unexpected(res.error());
            } else {
                fs::path dep_path = fs::path(build_dir) / "catalyst-libs" / name;
                if (fs::exists(dep_path)) {
                    std::println(std::cout, "Skipping fetch for existing git dependency: {}", name);
                } else {
                    if (!dep["version"] || !dep["version"].IsScalar()) {
                        return std::unexpected(std::format("git dependency '{}' is missing version.", name));
                    }
                    auto version = dep["version"].as<std::string>();
                    if (auto res = fetchGit(build_dir, name, source, version); !res)
                        return std::unexpected(res.error());
                }
            }
            ++ii;
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
