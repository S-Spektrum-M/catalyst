#include "catalyst/log-utils/log.hpp"
#include "catalyst/process_exec.h"
#include "catalyst/subcommands/build.hpp"
#include "catalyst/subcommands/generate.hpp"
#include "catalyst/yaml-utils/Configuration.hpp"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/yaml.h"

#include <array>
#include <cstdio>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <print>
#include <string>
#include <sys/wait.h>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace catalyst::generate {
namespace fs = std::filesystem;

void resolve_vcpkg_dependency(const YAML::Node &dep,
                              const std::string &triplet,
                              std::string &ldflags,
                              std::string &ldlibs) {
    const char *vcpkg_root_env = std::getenv("VCPKG_ROOT");
    if (vcpkg_root_env == nullptr) {
        catalyst::logger.log(LogLevel::WARN,
                             "VCPKG_ROOT is not set, cannot resolve vcpkg dependency '{}'.",
                             dep["name"].as<std::string>());
        return;
    }

    std::string dep_name = dep["name"].as<std::string>();
    catalyst::logger.log(LogLevel::DEBUG, "Resolving vcpkg dependency: {}", dep_name);

    // Construct the path to the library directory within the specific package folder
    // $VCPKG_ROOT/packages/<package>_<triplet>/lib
    fs::path vcpkg_root(vcpkg_root_env);
    fs::path package_dir_name = std::format("{}_{}", dep_name, triplet);
    fs::path lib_path = vcpkg_root / "packages" / package_dir_name / "lib";

    if (!fs::exists(lib_path) || !fs::is_directory(lib_path)) {
        catalyst::logger.log(LogLevel::WARN,
                             "Could not find library directory for vcpkg package '{}' at: {}",
                             dep_name,
                             lib_path.string());
        // Fallback: just add the library by name and hope the linker finds it in the global vcpkg lib dir.
        ldlibs += std::format(" -l{}", dep_name);
        return;
    }

    // Add this specific library path to the linker search paths.
    // This is often redundant if the global vcpkg lib path is already added, but it's more explicit.
    ldflags += std::format(" -L{}", lib_path.string());
    catalyst::logger.log(LogLevel::DEBUG, "Adding library path: {}", lib_path.string());

// Define the library file extensions based on the operating system.
#if defined(_WIN32)
    const std::vector<std::string> extensions = {".lib"};
#elif defined(__APPLE__)
    const std::vector<std::string> extensions = {".a", ".dylib"};
#else // Linux and other Unix-like systems
    const std::vector<std::string> extensions = {".a", ".so"};
#endif

    // Iterate through the directory and find matching library files.
    for (const auto &entry : fs::directory_iterator(lib_path)) {
        if (entry.is_regular_file()) {
            const fs::path &file_path = entry.path();
            std::string file_ext = file_path.extension().string();

            // Check if the file has one of the target extensions
            for (const auto &expected_ext : extensions) {
                if (file_ext == expected_ext) {
                    // Convert file path to a linker flag (e.g., "libfmt.a" -> "-lfmt")
                    std::string stem = file_path.stem().string();
                    if (stem.rfind("lib", 0) == 0) { // Check if it starts with "lib"
                        stem = stem.substr(3);
                    }
                    ldlibs += std::format(" -l{}", stem);
                    catalyst::logger.log(LogLevel::DEBUG, "Found and added library: {}", stem);
                    break; // Found a matching extension, move to the next file
                }
            }
        }
    }
}

std::expected<void, std::string> resolve_local_dependency(
    const YAML::Node &dep, std::string &cxxflags, std::string &ccflags, std::string &ldflags, std::string &ldlibs) {
    catalyst::logger.log(LogLevel::DEBUG, "Resolving local dependency: {}", dep["name"].as<std::string>());
    auto project_dir = fs::current_path();
    fs::path dep_path;
    if (dep["path"]) {
        dep_path = dep["path"].as<std::string>();
        catalyst::logger.log(LogLevel::DEBUG, "Changing directory to: {}", dep_path.string());
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
    catalyst::logger.log(LogLevel::DEBUG, "Composing profiles for local dependency.");
    auto pc = catalyst::generate::profile_composition(profiles);

    if (!pc) {
        fs::current_path(project_dir);
        return std::unexpected(pc.error());
    }
    YAML::Node profile = pc.value();

    catalyst::logger.log(LogLevel::DEBUG, "Building local dependency: {}.", dep["name"].as<std::string>());
    auto _ = catalyst::build::action(build::parse_t{
        .regen = true,
        .force_rebuild = true,
        .force_refetch = true,
        .profiles = profiles,
        .enabled_features = features,
    });
    catalyst::logger.log(LogLevel::DEBUG, "Changing directory back to: {}", project_dir.string());
    fs::current_path(project_dir);

    // Add include directories
    if (auto includes = profile["manifest"]["dirs"]["include"]; includes && includes.IsSequence()) {
        for (const auto &dir : includes.as<std::vector<std::string>>()) {
            auto include_path = fs::absolute(dep_path / dir);
            catalyst::logger.log(LogLevel::DEBUG, "Adding include path: {}", include_path.string());
            cxxflags += std::format(" -I{}", include_path.string());
            ccflags += std::format(" -I{}", include_path.string());
        }
    }

    // Add library directory
    if (auto build_dir_node = profile["manifest"]["dirs"]["build"]) {
        auto build_dir = build_dir_node.as<std::string>();
        auto lib_path = fs::absolute(dep_path / build_dir);
        catalyst::logger.log(LogLevel::DEBUG, "Adding library path: {}", lib_path.string());
        ldflags += std::format(" -L{}", lib_path.string());
    }

    // Add library
    if (auto dep_name_node = profile["manifest"]["name"]) {
        auto dep_name = dep_name_node.as<std::string>();
        catalyst::logger.log(LogLevel::DEBUG, "Adding library: {}", dep_name);
        ldlibs += std::format(" -l{}", dep_name);
    }

    return {};
}

void resolve_pkg_config_dependency(const YAML::Node &dep,
                                   std::string &cxxflags,
                                   std::string &ccflags,
                                   [[maybe_unused]] std::string &ldflags,
                                   std::string &ldlibs) {
    std::string dep_name = dep["name"].as<std::string>();
    catalyst::logger.log(LogLevel::DEBUG, "Resolving pkg-config dependency: {}", dep_name);

    if (dep["linkage"] && dep["linkage"].IsScalar()) {
        std::string linkage = dep["linkage"].as<std::string>();
        if (linkage == "static" || linkage == "shared") {
            ldlibs += " -l" + dep_name;
        }
        std::string cflags_command = std::format("pkg-config --cflags {}", dep_name);
        catalyst::logger.log(LogLevel::DEBUG, "Executing command: {}", cflags_command);
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
                catalyst::logger.log(LogLevel::DEBUG, "Adding cflags: {}", cflags_result);
                cxxflags += " " + cflags_result;
                ccflags += " " + cflags_result;
            }
        }
    } else {
        std::string result;
        if (auto res = process_exec_stdout({"pkg-config", "--libs", dep_name}); !res) {
            ldlibs += " -l" + dep_name;
            return;
        } else {
            result = *res;
        }
        result.erase(result.find_last_not_of(" \t\n") + 1);
        catalyst::logger.log(LogLevel::DEBUG, "Adding libs: {}", result);
        ldlibs += " " + result;
    }
}

void resolve_system_dependency(
    const YAML::Node &dep, std::string &cxxflags, std::string &ccflags, std::string &ldflags, std::string &ldlibs) {
    std::string dep_name = dep["name"].as<std::string>();
    catalyst::logger.log(LogLevel::DEBUG, "Resolving system dependency: {}", dep_name);
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
            catalyst::logger.log(LogLevel::DEBUG, "Assuming include path: {}", inc_path);
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
            catalyst::logger.log(LogLevel::DEBUG, "Assuming lib path: {}", lib_path);
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

// used to write to the buildfile.
void write_variables(const catalyst::YAML_UTILS::Configuration &config,
                     catalyst::generate::BuildWriters::BaseWriter &writer,
                     const std::vector<std::string> &enabled_features) {

    catalyst::logger.log(LogLevel::DEBUG, "Writing variables to build file.");
    fs::path current_dir = fs::current_path();
    std::string build_dir_str = config.get_string("manifest.dirs.build").value_or("build");
    fs::path build_dir{build_dir_str};
    fs::path obj_dir = "obj";

    std::string cxxflags =
                    std::format("{} -DCATALYST_BUILD_SYS=1 -DCATALYST_PROJ_NAME=\"{}\" -DCATALYST_PROJ_VER=\"{}\"",
                                config.get_string("manifest.tooling.CXXFLAGS").value_or(""),
                                config.get_string("manifest.name").value_or("name"),
                                config.get_string("manifest.version").value_or("0.0.0")),
                ccflags =
                    std::format("{} -DCATALYST_BUILD_SYS=1 -DCATALYST_PROJ_NAME=\"{}\" -DCATALYST_PROJ_VER=\"{}\"",
                                config.get_string("manifest.tooling.CCFLAGS").value_or(""),
                                config.get_string("manifest.name").value_or("name"),
                                config.get_string("manifest.version").value_or("0.0.0")),
                ldflags = "-Lcatalyst-libs";

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
        logger.log(LogLevel::WARN, "VCPKG_ROOT environment variable is not defined.");
    }

    if (const auto &features_node = config.get_root()["features"]; features_node && features_node.IsSequence()) {
        for (const auto &feature_map : features_node) {
            if (feature_map.IsMap()) {
                for (auto it = feature_map.begin(); it != feature_map.end(); ++it) {
                    std::string feature = it->first.as<std::string>();
                    bool default_enabled = it->second.as<bool>();
                    bool explicitly_enabled =
                        std::find(enabled_features.begin(), enabled_features.end(), feature) != enabled_features.end();
                    bool explicitly_disabled =
                        std::find(enabled_features.begin(), enabled_features.end(), "no-" + feature) !=
                        enabled_features.end();

                    bool is_enabled = default_enabled;
                    if (explicitly_enabled) {
                        is_enabled = true;
                    } else if (explicitly_disabled) {
                        is_enabled = false;
                    }

                    std::string flag = std::format(" -DFF_{}__{}={}",
                                                   config.get_string("manifest.name").value_or("name"),
                                                   feature,
                                                   is_enabled ? "1" : "0");
                    cxxflags += flag;
                    ccflags += flag;
                }
            }
            // reaching this is technically an error but we allow it
        }
    }

    std::vector<std::string> inc_dirs = config.get_string_vector("manifest.dirs.include").value();
    for (const auto &inc_dir : inc_dirs) {
        cxxflags += std::format(" -I{}", fs::absolute(inc_dir).string());
        ccflags += std::format(" -I{}", fs::absolute(inc_dir).string());
    }

    std::string ldlibs;
    for (const auto &dep : config.get_root()["dependencies"]) {
        if (auto res = find_dep(build_dir_str, dep); !res) {
            catalyst::logger.log(LogLevel::ERROR, "{}", res.error());
            continue;
        } else {
            auto [lib_path, inc_path, libs] = res.value();
            ldflags += " " + lib_path;
            ldlibs += " " + libs;
            ccflags += " " + inc_path;
            cxxflags += " " + inc_path;
        }
    }

    writer.add_comment("Variables");
    writer.add_variable("cc", config.get_string("manifest.tooling.CC").value_or("clang"));
    writer.add_variable("cxx", config.get_string("manifest.tooling.CXX").value_or("clang++"));
    writer.add_variable("cxxflags", cxxflags);
    writer.add_variable("cflags", ccflags);
    writer.add_variable("ldflags", ldflags);
    writer.add_variable("ldlibs", ldlibs); // place compiled libraries here
}

void write_rules(catalyst::generate::BuildWriters::BaseWriter &writer) {
    catalyst::logger.log(LogLevel::DEBUG, "Writing rules to build file.");
    writer.add_comment("Rules for compiling");
    writer.add_rule("cxx_compile", "$cxx $cxxflags -MMD -MF $out.d -c $in -o $out", "CXX $out", "$out.d", "gcc");
    writer.add_rule("cc_compile", "$cc $cflags -MMD -MF $out.d -c $in -o $out", "CC $out", "$out.d", "gcc");

    writer.add_comment("Rules for linking");
    writer.add_rule("binary_link", "$cxx $in -o $out $ldflags $ldlibs", "LINK $out");
    writer.add_rule("static_link", "ar rcs $out $in", "LINK $out");
    writer.add_rule("shared_link", "$cxx -shared $in -o $out", "LINK $out");
}
} // namespace catalyst::generate
