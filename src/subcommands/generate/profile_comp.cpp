#include "catalyst/log-utils/log.hpp"
#include "catalyst/subcommands/generate.hpp"
#include <expected>
#include <filesystem>
#include <string>
#include <sys/wait.h>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace catalyst::generate {

std::string ver_max(std::string s1, std::string s2) {
    catalyst::logger.log(LogLevel::INFO, "Comparing versions: {} and {}", s1, s2);
    auto split_ver = [](const std::string &ver) {
        std::vector<int> parts;
        std::istringstream iss(ver);
        for (std::string token; std::getline(iss, token, '.');) {
            try {
                parts.push_back(std::stoi(token));
            } catch (...) {
                parts.push_back(0);
            }
        }
        while (parts.size() < 3)
            parts.push_back(0);
        return parts;
    };

    auto v1 = split_ver(s1);
    auto v2 = split_ver(s2);

    for (size_t i = 0, lim = std::min(v1.size(), v2.size()); i < lim; ++i)
        if (v1[i] > v2[i]) {
            catalyst::logger.log(LogLevel::INFO, "Version {} is greater.", s1);
            return s1;
        } else if (v1[i] < v2[i]) {
            catalyst::logger.log(LogLevel::INFO, "Version {} is greater.", s2);
            return s2;
        } else
            continue;

    catalyst::logger.log(LogLevel::INFO, "Versions are equal, returning {}.", s1);
    return s1;
}

std::expected<YAML::Node, std::string> profile_composition(const std::vector<std::string> &p) {
    catalyst::logger.log(LogLevel::INFO, "Composing profiles.");
    std::vector profiles = p;
    if (std::find(profiles.begin(), profiles.end(), "common") == profiles.end())
        profiles.insert(profiles.cbegin(), std::string{"common"});
    // define with defaults and add onto this
    YAML::Node composite;
    composite["meta"]["min_ver"] = "0.0.1";
    composite["manifest"]["name"] = "name";
    composite["manifest"]["type"] = "BINARY";
    composite["manifest"]["version"] = "0.0.1";
    composite["manifest"]["provides"] = "";
    composite["manifest"]["tooling"]["CC"] = "clang";
    composite["manifest"]["tooling"]["CXX"] = "clang++";
    composite["manifest"]["tooling"]["FMT"] = "clang-format";
    composite["manifest"]["tooling"]["LINTER"] = "clang-tidy";
    composite["manifest"]["tooling"]["CCFLAGS"] = "";
    composite["manifest"]["tooling"]["CXXFLAGS"] = "";
    composite["manifest"]["dirs"]["include"] = std::vector<std::string>{};
    composite["manifest"]["dirs"]["source"] = std::vector<std::string>{};
    composite["manifest"]["dirs"]["build"] = "";
    composite["dependencies"] = std::vector<YAML::Node>{};
    composite["features"] = std::vector<std::string>{};
    namespace fs = std::filesystem;
    std::vector<fs::path> profile_paths;
    for (const std::string &profile_name : profiles) {
        fs::path path{};
        if (profile_name == "common")
            profile_paths.push_back("catalyst.yaml");
        else
            profile_paths.push_back(std::format("catalyst_{}.yaml", profile_name));
    }
    for (const auto &path : profile_paths) {
        catalyst::logger.log(LogLevel::INFO, "Loading profile: {}", path.string());
        if (!fs::exists(path)) {
            catalyst::logger.log(LogLevel::ERROR, "Profile not found: {}", path.string());
            return std::unexpected(std::format("Profile: {} not found", path.string()));
        }
        auto new_profile = YAML::LoadFile(path);
        if (new_profile["meta"]["min_ver"].IsDefined()) {
            composite["meta"]["min_ver"] = ver_max(composite["meta"]["min_ver"].as<std::string>(),
                                                   new_profile["meta"]["min_ver"].as<std::string>());
        }
        if (YAML::Node new_profile_manifest = new_profile["manifest"]; new_profile_manifest.IsDefined()) {
            if (new_profile_manifest["name"]) {
                composite["manifest"]["name"] = new_profile_manifest["name"];
            }
            if (new_profile_manifest["type"]) {
                composite["manifest"]["type"] = new_profile_manifest["type"];
            }
            if (new_profile_manifest["version"]) {
                composite["manifest"]["version"] = new_profile_manifest["version"];
            }
            if (new_profile_manifest["provides"]) {
                composite["manifest"]["provides"] = new_profile_manifest["provides"];
            }
            if (YAML::Node new_profile_manifest_tooling = new_profile_manifest["tooling"];
                new_profile_manifest_tooling) {
                if (new_profile_manifest_tooling["CC"]) {
                    composite["manifest"]["tooling"]["CC"] = new_profile_manifest_tooling["CC"];
                }
                if (new_profile_manifest_tooling["CXX"]) {
                    composite["manifest"]["tooling"]["CXX"] = new_profile_manifest_tooling["CXX"];
                }
                if (new_profile_manifest_tooling["FMT"]) {
                    composite["manifest"]["tooling"]["FMT"] = new_profile_manifest_tooling["FMT"];
                }
                if (new_profile_manifest_tooling["LINTER"]) {
                    composite["manifest"]["tooling"]["LINTER"] = new_profile_manifest_tooling["LINTER"];
                }
                if (new_profile_manifest_tooling["CCFLAGS"]) {
                    composite["manifest"]["tooling"]["CCFLAGS"] = new_profile_manifest_tooling["CCFLAGS"];
                }
                if (new_profile_manifest_tooling["CXXFLAGS"]) {
                    composite["manifest"]["tooling"]["CXXFLAGS"] = new_profile_manifest_tooling["CXXFLAGS"];
                }
            }
            if (YAML::Node new_profile_manifest_dirs = new_profile_manifest["dirs"]; new_profile_manifest_dirs) {
                if (new_profile_manifest_dirs["include"] && new_profile_manifest_dirs["include"].IsSequence())
                    for (auto inc : new_profile_manifest_dirs["include"].as<std::vector<std::string>>())
                        composite["manifest"]["dirs"]["include"].push_back(inc);
                if (new_profile_manifest_dirs["source"] && new_profile_manifest_dirs["source"].IsSequence())
                    for (auto src : new_profile_manifest_dirs["source"].as<std::vector<std::string>>())
                        composite["manifest"]["dirs"]["source"].push_back(src);
                if (new_profile_manifest_dirs["build"])
                    composite["manifest"]["dirs"]["build"] = new_profile_manifest_dirs["build"];
            }
        }
        if (new_profile["features"] && new_profile["features"].IsSequence()) {
            for (const auto &feature : new_profile["features"]) {
                composite["features"].push_back(feature.as<std::string>());
            }
        }
        if (new_profile["dependencies"] && new_profile["dependencies"].IsSequence()) {
            for (const YAML::Node &dep : new_profile["dependencies"]) {
                composite["dependencies"].push_back(dep);
            }
        }
        if (new_profile["hooks"]) {
            for (const auto &hook : new_profile["hooks"]) {
                std::string hook_name = hook.first.as<std::string>();
                if (hook.second.IsSequence()) {
                    for (const auto &item : hook.second) {
                        composite["hooks"][hook_name].push_back(item);
                    }
                } else if (hook.second.IsScalar()) {
                    composite["hooks"][hook_name].push_back(hook.second);
                }
            }
        }
    }
    catalyst::logger.log(LogLevel::INFO, "Profile composition finished.");
    return composite;
}
} // namespace catalyst::generate
