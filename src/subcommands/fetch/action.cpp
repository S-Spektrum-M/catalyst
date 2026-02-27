#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
#include <iostream>
#include <print>
#include <sstream>
#include <string>
#include <unordered_map>

#include <yaml-cpp/yaml.h>

#include "catalyst/hooks.hpp"
#include "catalyst/utils/log/log.hpp"
#include "catalyst/process_exec.hpp"
#include "catalyst/subcommands/fetch.hpp"

namespace catalyst::fetch {
namespace fs = std::filesystem;

namespace {

std::expected<void, std::string> fetchVcpkg(const std::string &name) {
    catalyst::logger.log(LogLevel::DEBUG, "Fetching vcpkg dependency: {}", name);
    char *vcpkg_root_env = std::getenv("VCPKG_ROOT");
    if (vcpkg_root_env == nullptr) {
        catalyst::logger.log(LogLevel::ERROR, "VCPKG_ROOT environment variable not set.");
        return std::unexpected(
            "VCPKG_ROOT environment variable not set. Please set it to your vcpkg installation directory.");
    }
    fs::path vcpkg_root(vcpkg_root_env);
    fs::path vcpkg_exe = vcpkg_root / "vcpkg";
#if defined(_WIN32)
    vcpkg_exe.replace_extension(".exe");
#endif
    std::string command = std::format("\"{}\" install {}", vcpkg_exe.string(), name);
    catalyst::logger.log(LogLevel::DEBUG, "Executing command: {}", command);
    catalyst::logger.log(LogLevel::DEBUG, "Fetching: {} from vcpkg", name);
    if (catalyst::processExec({vcpkg_exe.string(), "install", name}).value().get() != 0) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to fetch dependency: {}", name);
        return std::unexpected(std::format("Failed to fetch dependency: {}", name));
    }
    return {};
}

std::expected<void, std::string>
fetchGit(std::string build_dir, std::string name, std::string source, std::string version) {
    catalyst::logger.log(LogLevel::DEBUG, "Fetching git dependency: {}@{} from {}", name, version, source);
    fs::path dep_path = fs::path(build_dir) / "catalyst-libs" / name;
    std::println(std::cout, "Fetching: {}@{} from {}", name, version, source);
    std::string command;
    std::vector<std::string> args = {"git", "clone", "--depth", "1"};
    if (version == "latest") {
        command = std::format("git clone --depth 1 {} {}", source, dep_path.string());
        args.push_back(source);
        args.push_back(dep_path.string());
    } else {
        command = std::format("git clone --depth 1 --branch {} {} {}", version, source, dep_path.string());
        args.push_back("--branch");
        args.push_back(version);
        args.push_back(source);
        args.push_back(dep_path.string());
    }
    catalyst::logger.log(LogLevel::DEBUG, "Executing command: {}", command);
    if (catalyst::processExec(std::move(args)).value().get() != 0) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to fetch dependency: {}", name);
        return std::unexpected(std::format("Failed to fetch dependency: {}", name));
    }
    return {};
}

std::expected<void, std::string> fetchSystem(const std::string &name) {
    // assuming installed on system
    catalyst::logger.log(LogLevel::DEBUG, "Skipping fetch for system dependency: {}", name);
    return {};
}

std::expected<void, std::string>
fetchLocal(const std::string &name, const std::string &path, const std::vector<std::string> &profiles) {
    fs::path local_path = fs::absolute(path);
    std::string visited_env = std::getenv("CATALYST_VISITED") ? std::getenv("CATALYST_VISITED") : "";

    // Cycle detection
    std::stringstream ss(visited_env);
    std::string segment;
    while (std::getline(ss, segment, ':')) {
        if (!segment.empty() && fs::equivalent(fs::path(segment), local_path)) {
            return std::unexpected(std::format("Dependency cycle detected involving {}", local_path.string()));
        }
    }

    std::string new_visited = visited_env.empty() ? local_path.string() : visited_env + ":" + local_path.string();

    catalyst::logger.log(LogLevel::DEBUG, "Recursively building local dependency: {} at {}", name, local_path.string());
    std::println(std::cout, "Building local dependency: {} at {}", name, local_path.string());

    std::vector<std::string> args = {"catalyst", "build"};
    if (profiles.size() != 0) {
        args.push_back("--profiles");
        for (const auto &p : profiles) {
            args.push_back(p);
        }
    }

    std::unordered_map<std::string, std::string> env_map;
    env_map["CATALYST_VISITED"] = new_visited;

    auto res = catalyst::processExec(std::move(args), local_path.string(), env_map);
    if (!res) {
        return std::unexpected(res.error());
    }

    if (res.value().get() != 0) {
        return std::unexpected(std::format("Failed to build local dependency: {}", name));
    }

    return {};
}

} // namespace

std::expected<void, std::string> action(const Parse &parse_args) {
    catalyst::logger.log(LogLevel::DEBUG, "Fetch subcommand invoked.");
    catalyst::logger.log(LogLevel::DEBUG, "Composing profiles.");
    utils::yaml::Configuration config{parse_args.profiles};

    catalyst::logger.log(LogLevel::DEBUG, "Running pre-fetch hooks.");
    if (auto res = hooks::preFetch(config); !res) {
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
                if (auto member = parse_args.workspace->findPackage(name)) {
                    catalyst::logger.log(LogLevel::INFO,
                                         "Dependency '{}' found in workspace at '{}'. Linking...",
                                         name,
                                         member->path.string());
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
                    } catch (const std::exception &e) {
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
    if (auto res = hooks::postFetch(config); !res) {
        catalyst::logger.log(LogLevel::ERROR, "Post-fetch hook failed: {}", res.error());
        return res;
    }

    catalyst::logger.log(LogLevel::DEBUG, "Fetch subcommand finished successfully.");
    return {};
}
} // namespace catalyst::fetch
