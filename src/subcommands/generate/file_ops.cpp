#include "catalyst/log-utils/log.hpp"
#include "catalyst/subcommands/build.hpp"
#include "catalyst/subcommands/generate.hpp"
#include "yaml-cpp/node/node.h"
#include <array>
#include <cstdio>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <sys/wait.h>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace catalyst::generate {
namespace fs = std::filesystem;

static std::expected<void, std::string> resolve_local_dependency(const YAML::Node &dep, std::string &cxxflags,
                                                                 std::string &ccflags, std::string &ldflags,
                                                                 std::string &ldlibs) {
    catalyst::logger.log(LogLevel::INFO, "Resolving local dependency: {}", dep["name"].as<std::string>());
    auto project_dir = fs::current_path();
    fs::path dep_path;
    if (dep["path"]) {
        dep_path = dep["path"].as<std::string>();
        catalyst::logger.log(LogLevel::INFO, "Changing directory to: {}", dep_path.string());
        fs::current_path(dep_path);
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

    if (!pc) {
        fs::current_path(project_dir);
        return std::unexpected(pc.error());
    }
    YAML::Node profile = pc.value();

    catalyst::logger.log(LogLevel::INFO, "Building local dependency.");
    auto _ = catalyst::build::action(build::parse_t{
        .regen = true,
        .force_rebuild = true,
        .force_refetch = true,
        .profiles = profiles,
        .enabled_features = features,
    });
    catalyst::logger.log(LogLevel::INFO, "Changing directory back to: {}", project_dir.string());
    fs::current_path(project_dir);

    // Add include directories
    if (auto includes = profile["manifest"]["dirs"]["include"]; includes && includes.IsSequence()) {
        for (const auto &dir : includes.as<std::vector<std::string>>()) {
            auto include_path = fs::absolute(dep_path / dir);
            catalyst::logger.log(LogLevel::INFO, "Adding include path: {}", include_path.string());
            cxxflags += std::format(" -I{}", include_path.string());
            ccflags += std::format(" -I{}", include_path.string());
        }
    }

    // Add library directory
    if (auto build_dir_node = profile["manifest"]["dirs"]["build"]) {
        auto build_dir = build_dir_node.as<std::string>();
        auto lib_path = fs::absolute(dep_path / build_dir);
        catalyst::logger.log(LogLevel::INFO, "Adding library path: {}", lib_path.string());
        ldflags += std::format(" -L{}", lib_path.string());
    }

    // Add library
    if (auto dep_name_node = profile["manifest"]["name"]) {
        auto dep_name = dep_name_node.as<std::string>();
        catalyst::logger.log(LogLevel::INFO, "Adding library: {}", dep_name);
        ldlibs += std::format(" -l{}", dep_name);
    }

    return {};
}

static void resolve_pkg_config_dependency(const YAML::Node &dep, std::string &cxxflags, std::string &ccflags,
                                          [[maybe_unused]] std::string &ldflags, std::string &ldlibs) {
    std::string dep_name = dep["name"].as<std::string>();
    catalyst::logger.log(LogLevel::INFO, "Resolving pkg-config dependency: {}", dep_name);

    if (dep["linkage"] && dep["linkage"].IsScalar()) {
        std::string linkage = dep["linkage"].as<std::string>();
        if (linkage == "static" || linkage == "shared") {
            ldlibs += " -l" + dep_name;
        }
        std::string cflags_command = std::format("pkg-config --cflags {}", dep_name);
        catalyst::logger.log(LogLevel::INFO, "Executing command: {}", cflags_command);
        std::array<char, 128> cflags_buffer;
        std::string cflags_result;
        FILE *cflags_pipe = popen(cflags_command.c_str(), "r");
        if (cflags_pipe) {
            while (fgets(cflags_buffer.data(), cflags_buffer.size(), cflags_pipe) != nullptr) {
                cflags_result += cflags_buffer.data();
            }
            int cflags_ret = pclose(cflags_pipe);
            if (WEXITSTATUS(cflags_ret) == 0) {
                cflags_result.erase(cflags_result.find_last_not_of(" \t\n") + 1);
                catalyst::logger.log(LogLevel::INFO, "Adding cflags: {}", cflags_result);
                cxxflags += " " + cflags_result;
                ccflags += " " + cflags_result;
            }
        }
    } else {
        std::string command = std::format("pkg-config --libs {}", dep_name);
        catalyst::logger.log(LogLevel::INFO, "Executing command: {}", command);
        std::array<char, 128> buffer;
        std::string result;
        FILE *pipe = popen(command.c_str(), "r");
        if (!pipe) {
            ldlibs += " -l" + dep_name;
            return;
        }
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
        int ret = pclose(pipe);
        if (WEXITSTATUS(ret) == 0) {
            result.erase(result.find_last_not_of(" \t\n") + 1);
            catalyst::logger.log(LogLevel::INFO, "Adding libs: {}", result);
            ldlibs += " " + result;
        } else {
            ldlibs += " -l" + dep_name;
        }
    }
}

static void resolve_system_dependency(const YAML::Node &dep, std::string &cxxflags, std::string &ccflags,
                                      std::string &ldflags, std::string &ldlibs) {
    std::string dep_name = dep["name"].as<std::string>();
    catalyst::logger.log(LogLevel::INFO, "Resolving system dependency: {}", dep_name);
    bool used_explicit_paths = false;

    std::string linkage = "shared";
    if (dep["linkage"] && dep["linkage"].IsScalar()) {
        linkage = dep["linkage"].as<std::string>();
    }

    if (dep["include"]) {
        cxxflags += std::format(" -I{}", dep["include"].as<std::string>());
        ccflags += std::format(" -I{}", dep["include"].as<std::string>());
        used_explicit_paths = true;
    } else {
        std::string inc_path;
#if defined(_WIN32)
#elif defined(__APPLE__)
        inc_path = "-I/usr/local/include";
#else
        inc_path = "-I/usr/include";
#endif
        if (!inc_path.empty()) {
            catalyst::logger.log(LogLevel::INFO, "Assuming include path: {}", inc_path);
            ccflags += " " + inc_path;
            cxxflags += " " + inc_path;
        }
    }

    if (dep["lib"]) {
        ldflags += std::format(" -L{}", dep["lib"].as<std::string>());
        used_explicit_paths = true;
    } else {
        std::string lib_path;
#if defined(_WIN32)
#elif defined(__APPLE__)
        lib_path = "-L/usr/local/lib";
#else
        lib_path = "-L/usr/lib";
#endif
        if (!lib_path.empty()) {
            catalyst::logger.log(LogLevel::INFO, "Assuming lib path: {}", lib_path);
            ldflags += " " + lib_path;
        }
    }

    if (used_explicit_paths) {
        if (linkage == "static" || linkage == "shared") {
            ldlibs += " -l" + dep_name;
        }
    } else {
        resolve_pkg_config_dependency(dep, cxxflags, ccflags, ldflags, ldlibs);
    }
}

void write_variables(const YAML::Node &profile, std::ofstream &buildfile,
                     const std::vector<std::string> &enabled_features) {
    catalyst::logger.log(LogLevel::INFO, "Writing variables to build file.");
    fs::path current_dir = fs::current_path();
    std::string build_dir_str = profile["manifest"]["dirs"]["build"].as<std::string>();
    fs::path build_dir(build_dir_str);
    fs::path obj_dir = "obj";

    std::string cxxflags =
        std::format("{} -DCATALYST_BUILD_SYS=1 -DCATALYST_PROJ_NAME=\"{}\" -DCATALYST_PROJ_VER=\"{}\"",
                    profile["manifest"]["tooling"]["CXXFLAGS"].as<std::string>(),
                    profile["manifest"]["name"].as<std::string>(), profile["manifest"]["version"].as<std::string>());
    std::string ccflags =
        std::format("{} -DCATALYST_BUILD_SYS=1 -DCATALYST_PROJ_NAME=\"{}\" -DCATALYST_PROJ_VER=\"{}\"",
                    profile["manifest"]["tooling"]["CCFLAGS"].as<std::string>(),
                    profile["manifest"]["name"].as<std::string>(), profile["manifest"]["version"].as<std::string>());

    std::string ldflags = "-Lcatalyst-libs";

    if (const char *vcpkg_root = std::getenv("VCPKG_ROOT"); vcpkg_root != nullptr) {
#if defined(_WIN32)
        const char *triplet = "x64-windows";
#elif defined(__APPLE__)
        const char *triplet = "x64-osx";
#else
        const char *triplet = "x64-linux";
#endif
        cxxflags += std::format(" -I{}", (fs::path(vcpkg_root) / "installed" / triplet / "include").string());
        ccflags += std::format(" -I{}", (fs::path(vcpkg_root) / "installed" / triplet / "include").string());
        ldflags += std::format(" -L{}", (fs::path(vcpkg_root) / "installed" / triplet / "lib").string());
    } else {
        logger.log(LogLevel::INFO, "VCPKG_ROOT environment variable is not defined.");
    }

    // Add feature flags
    if (profile["features"]) {
        for (const auto &feature : profile["features"]) {
            std::string feature_name = feature.as<std::string>();
            std::string project_name = profile["manifest"]["name"].as<std::string>();
            bool is_enabled =
                std::find(enabled_features.begin(), enabled_features.end(), feature_name) != enabled_features.end();
            std::string flag = std::format(" -DFF_{}__{}={}", project_name, feature_name, is_enabled ? "1" : "0");
            cxxflags += flag;
            ccflags += flag;
        }
    }

    if (auto includes = profile["manifest"]["dirs"]["include"]; includes && includes.IsSequence()) {
        for (const auto &dir : includes.as<std::vector<std::string>>()) {
            cxxflags += std::format(" -I{}", fs::absolute(dir).string());
            ccflags += std::format(" -I{}", fs::absolute(dir).string());
        }
    }

    std::string ldlibs;
    if (auto deps = profile["dependencies"]; deps && deps.IsSequence()) {
        for (const auto &dep : deps) {
            if (!dep["name"] || !dep["name"].IsScalar())
                continue;

            std::string source = dep["source"] ? dep["source"].as<std::string>() : "";

            if (source == "system") {
                resolve_system_dependency(dep, cxxflags, ccflags, ldflags, ldlibs);
            } else if (source == "local") {
                if (auto res = resolve_local_dependency(dep, cxxflags, ccflags, ldflags, ldlibs); !res) {
                    logger.log(LogLevel::ERROR, "Failed to resolve local dependency {}: {}",
                               dep["name"].as<std::string>(), res.error());
                }
            } else {
                resolve_pkg_config_dependency(dep, cxxflags, ccflags, ldflags, ldlibs);
            }
        }
    }

    buildfile << "# Variables\n"
              << "cc = " << profile["manifest"]["tooling"]["CC"].as<std::string>() << "\n"
              << "cxx = " << profile["manifest"]["tooling"]["CXX"].as<std::string>() << "\n"
              << "cxxflags = " << cxxflags << "\n"
              << "cflags = " << ccflags << "\n"
              << "builddir = " << build_dir.string() << "\n"
              << "objdir = " << obj_dir.string() << "\n"
              << "ldflags = " << ldflags << " \n"
              << "ldlibs= " << ldlibs << "\n\n"; // place compiled libraries here
}

void write_rules(std::ofstream &buildfile) {
    catalyst::logger.log(LogLevel::INFO, "Writing rules to build file.");
    buildfile << "# Rules for compiling\n";
    buildfile << "rule cxx_compile\n";
    buildfile << "  command = $cxx $cxxflags -MD -MF $out.d -c $in -o $out\n";
    buildfile << "  description = CXX $out\n";
    buildfile << "  depfile = $out.d\n";
    buildfile << "  deps = gcc\n\n";

    buildfile << "rule c_compile\n";
    buildfile << "  command = $cc $cflags -MD -MF $out.d -c $in -o $out\n";
    buildfile << "  description = CC $out\n";
    buildfile << "  depfile = $out.d\n";
    buildfile << "  deps = gcc\n\n";

    buildfile << "# Rules for linking\n";
    buildfile << "rule binary_link\n";
    buildfile << "  command = $cxx $in -o $out $ldflags $ldlibs\n";
    buildfile << "  description = LINK $out\n\n";

    buildfile << "rule static_link\n";
    buildfile << "  command = ar rcs $out $in\n";
    buildfile << "  description = LINK $out\n\n";

    buildfile << "rule shared_link\n";
    buildfile << "  command = $cxx -shared $in -o $out\n";
    buildfile << "  description = LINK $out\n\n";
}
} // namespace catalyst::generate
