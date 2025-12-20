// a helper function to just get ldlibs for use in run/action.cpp
#include "catalyst/log-utils/log.hpp"
#include "catalyst/subcommands/generate.hpp"
#include "yaml-cpp/yaml.h"

#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
namespace catalyst::generate {

std::string ld_filter(std::string &ldflags);

// NOTE: used for run::action. Needs to be updated to use find_*.
std::expected<std::string, std::string> lib_path(const YAML::Node &profile) {
    catalyst::logger.log(LogLevel::DEBUG, "Writing variables to build file.");
    fs::path current_dir = fs::current_path();
    fs::path build_dir{profile["manifest"]["dirs"]["build"].as<std::string>()};
    fs::path obj_dir = "obj";

    std::string cxxflags{""}; // irrelevant for this function
    std::string ccflags{""};  // irrelevant for this function
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
        logger.log(LogLevel::WARN, "VCPKG_ROOT environment variable is not defined.");
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
                    logger.log(LogLevel::ERROR,
                               "Failed to resolve local dependency {}: {}",
                               dep["name"].as<std::string>(),
                               res.error());
                }
            } else if (source == "vcpkg") {
                if (!dep["triplet"] || !dep["triplet"].IsScalar()) {
                    catalyst::logger.log(LogLevel::ERROR,
                                         "vcpkg dependency: {} does not define field: triplet",
                                         dep["name"].as<std::string>());
                    return std::unexpected("unable to calculate ld_lib_path");
                }
                auto triplet = dep["triplet"].as<std::string>();
                resolve_vcpkg_dependency(dep, triplet, ldflags, ldlibs);
            } else {
                // an emulation of a local node
                YAML::Node node;
                std::string name = dep["name"].as<std::string>();
                node["name"] = dep["name"];
                node["path"] = fs::path{fs::path(build_dir) / "catalyst-libs" / name}.string();
                // TODO: check if these actually exist
                node["using"] = dep["using"];
                if (dep["profiles"] && dep["profiles"].IsSequence())
                    node["profiles"] = dep["profiles"];
                if (dep["using"] && dep["using"].IsSequence())
                    node["using"] = dep["using"];
                if (auto res = resolve_local_dependency(node, cxxflags, ccflags, ldflags, ldlibs); !res) {
                    logger.log(LogLevel::ERROR,
                               "Failed to resolve git dependency {}: {}",
                               node["name"].as<std::string>(),
                               res.error());
                }
            }
        }
    }
    return ld_filter(ldflags);
}

std::string ld_filter(std::string &ldflags) {
    std::stringstream ss(ldflags);
    std::string item;
    std::vector<std::string> tokens;
    while (std::getline(ss, item, ' ')) {
        if (item.rfind("-L", 0) == 0) {
            tokens.push_back(fs::absolute(item.substr(2)).string());
        }
    }

    std::string result;
    for (size_t i = 0; i < tokens.size(); ++i) {
        result += tokens[i];
        if (i < tokens.size() - 1) {
#if defined(_WIN32)
            result += ";";
#else
            result += ":";
#endif
        }
    }
    return result;
}
} // namespace catalyst::generate