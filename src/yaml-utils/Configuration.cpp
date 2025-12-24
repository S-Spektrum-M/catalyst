#include "catalyst/yaml-utils/Configuration.hpp"

#include "catalyst/log-utils/log.hpp"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/parse.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <format>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

using catalyst::LogLevel;
using namespace catalyst::YAML_UTILS;
namespace fs = std::filesystem;

namespace {
YAML::Node getDefaultConfiguration() {
    YAML::Node root;
    root["meta"]["min_ver"] = "0.0.1";
    root["manifest"]["name"] = "name";
    root["manifest"]["type"] = "BINARY";
    root["manifest"]["version"] = "0.0.1";
    root["manifest"]["provides"] = "";
    root["manifest"]["tooling"]["CC"] = "clang";
    root["manifest"]["tooling"]["CXX"] = "clang++";
    root["manifest"]["tooling"]["FMT"] = "clang-format";
    root["manifest"]["tooling"]["LINTER"] = "clang-tidy";
    root["manifest"]["tooling"]["CCFLAGS"] = "";
    root["manifest"]["tooling"]["CXXFLAGS"] = "";
    root["manifest"]["dirs"]["include"] = std::vector<std::string>{};
    root["manifest"]["dirs"]["source"] = std::vector<std::string>{};
    root["manifest"]["dirs"]["build"] = "";
    root["dependencies"] = std::vector<YAML::Node>{};
    root["features"] = std::vector<std::string>{};
    return root;
}
std::string ver_max(std::string s1, std::string s2) {
    catalyst::logger.log(LogLevel::DEBUG, "Comparing versions: {} and {}", s1, s2);
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
            catalyst::logger.log(LogLevel::DEBUG, "Version {} is greater.", s1);
            return s1;
        } else if (v1[i] < v2[i]) {
            catalyst::logger.log(LogLevel::DEBUG, "Version {} is greater.", s2);
            return s2;
        } else
            continue;

    catalyst::logger.log(LogLevel::DEBUG, "Versions are equal, returning {}.", s1);
    return s1;
}

void merge(YAML::Node &composite, const fs::path &profile_path) {
    YAML::Node new_profile = YAML::LoadFile(profile_path);
    YAML::Node defaults = getDefaultConfiguration();

    auto check_conflict = [&](const std::string &section, const std::string &key, const std::string &subkey,
                              const std::string &incoming_val) {
        try {
            std::string current_val;
            std::string default_val;

            if (subkey.empty()) {
                current_val = composite[section][key].as<std::string>();
                default_val = defaults[section][key].as<std::string>();
            } else {
                current_val = composite[section][key][subkey].as<std::string>();
                default_val = defaults[section][key][subkey].as<std::string>();
            }

            if (current_val != incoming_val && current_val != default_val) {
                std::string full_key = section + "." + key + (subkey.empty() ? "" : "." + subkey);
                catalyst::logger.log(LogLevel::WARN, "Profile '{}' overrides '{}': '{}' -> '{}'",
                                     profile_path.string(), full_key, current_val, incoming_val);
            }
        } catch (...) {
            // Ignore errors
        }
    };

    if (new_profile["meta"]["min_ver"].IsDefined()) {
        composite["meta"]["min_ver"] =
            ver_max(composite["meta"]["min_ver"].as<std::string>(), new_profile["meta"]["min_ver"].as<std::string>());
    }

    if (YAML::Node new_profile_manifest = new_profile["manifest"]; new_profile_manifest.IsDefined()) {
        if (new_profile_manifest["name"]) {
            check_conflict("manifest", "name", "", new_profile_manifest["name"].as<std::string>());
            composite["manifest"]["name"] = new_profile_manifest["name"];
        }
        if (new_profile_manifest["type"]) {
            check_conflict("manifest", "type", "", new_profile_manifest["type"].as<std::string>());
            composite["manifest"]["type"] = new_profile_manifest["type"];
        }
        if (new_profile_manifest["version"]) {
            check_conflict("manifest", "version", "", new_profile_manifest["version"].as<std::string>());
            composite["manifest"]["version"] = new_profile_manifest["version"];
        }
        if (new_profile_manifest["provides"]) {
            check_conflict("manifest", "provides", "", new_profile_manifest["provides"].as<std::string>());
            composite["manifest"]["provides"] = new_profile_manifest["provides"];
        }
        if (YAML::Node new_profile_manifest_tooling = new_profile_manifest["tooling"]; new_profile_manifest_tooling) {
            if (new_profile_manifest_tooling["CC"]) {
                check_conflict("manifest", "tooling", "CC", new_profile_manifest_tooling["CC"].as<std::string>());
                composite["manifest"]["tooling"]["CC"] = new_profile_manifest_tooling["CC"];
            }
            if (new_profile_manifest_tooling["CXX"]) {
                check_conflict("manifest", "tooling", "CXX", new_profile_manifest_tooling["CXX"].as<std::string>());
                composite["manifest"]["tooling"]["CXX"] = new_profile_manifest_tooling["CXX"];
            }
            if (new_profile_manifest_tooling["FMT"]) {
                check_conflict("manifest", "tooling", "FMT", new_profile_manifest_tooling["FMT"].as<std::string>());
                composite["manifest"]["tooling"]["FMT"] = new_profile_manifest_tooling["FMT"];
            }
            if (new_profile_manifest_tooling["LINTER"]) {
                check_conflict("manifest", "tooling", "LINTER",
                               new_profile_manifest_tooling["LINTER"].as<std::string>());
                composite["manifest"]["tooling"]["LINTER"] = new_profile_manifest_tooling["LINTER"];
            }
            if (new_profile_manifest_tooling["CCFLAGS"]) {
                check_conflict("manifest", "tooling", "CCFLAGS",
                               new_profile_manifest_tooling["CCFLAGS"].as<std::string>());
                composite["manifest"]["tooling"]["CCFLAGS"] = new_profile_manifest_tooling["CCFLAGS"];
            }
            if (new_profile_manifest_tooling["CXXFLAGS"]) {
                check_conflict("manifest", "tooling", "CXXFLAGS",
                               new_profile_manifest_tooling["CXXFLAGS"].as<std::string>());
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
            if (new_profile_manifest_dirs["build"]) {
                check_conflict("manifest", "dirs", "build", new_profile_manifest_dirs["build"].as<std::string>());
                composite["manifest"]["dirs"]["build"] = new_profile_manifest_dirs["build"];
            }
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
    if (new_profile["hooks"] && new_profile["hooks"].IsDefined()) {
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
std::vector<std::string> split_path(const std::string &key) {
    std::stringstream ss(key);
    std::string segment;
    std::vector<std::string> segments;
    while (std::getline(ss, segment, '.')) {
        segments.push_back(segment);
    }
    return segments;
}
std::optional<YAML::Node> traverse(const std::string &key, YAML::Node &&root) {
    auto segments = split_path(key);
    YAML::Node current = YAML::Node(root);

    for (const auto &s : segments) {
        if (!current[s]) {
            return std::nullopt;
        }
        current = current[s];
    }
    return current;
}
} // namespace

Configuration::Configuration(const std::vector<std::string> &profiles) {
    catalyst::logger.log(LogLevel::DEBUG, "Composing profiles.");

    std::vector profile_names = profiles;
    if (std::find(profile_names.begin(), profile_names.end(), "common") == profile_names.end())
        profile_names.insert(profile_names.cbegin(), std::string{"common"});

    root = getDefaultConfiguration();

    // This is possibly better than creating a temporary std::unordered_set
    for (size_t ii = 0; ii < profiles.size(); ++ii) {
        for (size_t jj = 0; jj < ii; ++jj) {
            if (profiles[jj] == profiles[ii]) {
                throw std::runtime_error(std::format(
                    "Duplicate profiles: {0} at index {1} and {0} at index {2}", profiles[ii], ii, jj
                ));
            }
        }
    }

    for (const auto &profile_name : profile_names) {
        fs::path profile_path{};
        if (profile_name == "common")
            profile_path = "catalyst.yaml";
        else
            profile_path = std::format("catalyst_{}.yaml", profile_name);
        if (!fs::exists(profile_path)) {
            catalyst::logger.log(LogLevel::ERROR, "Profile not found: {}", profile_path.string());
            throw std::runtime_error(std::format("Profile: {} not found", profile_path.string()));
        }
        auto new_profile = YAML::LoadFile(profile_path);
        merge(root, profile_path);
    }

    catalyst::logger.log(LogLevel::DEBUG, "Profile composition finished.");
}

bool Configuration::has(const std::string &key) const {
    auto segments = split_path(key);
    YAML::Node current = YAML::Clone(root);

    for (const auto &segment : segments) {
        if (!current[segment]) {
            return false;
        }
        current = current[segment];
    }
    return true;
}

std::optional<std::string> Configuration::get_string(const std::string &key) const {
    YAML::Node final;
    if (auto res = traverse(key, YAML::Clone(root)); !res) {
        return std::nullopt;
    } else {
        final = res.value();
    }

    try {
        return final.as<std::string>();
    } catch (const YAML::Exception &) {
        return std::nullopt;
    }
}

std::optional<int> Configuration::get_int(const std::string &key) const {
    YAML::Node final;
    if (auto res = traverse(key, YAML::Clone(root)); !res) {
        return std::nullopt;
    } else {
        final = res.value();
    }

    try {
        return final.as<int>();
    } catch (const YAML::Exception &) {
        return std::nullopt;
    }
}

std::optional<bool> Configuration::get_bool(const std::string &key) const {
    YAML::Node final;
    if (auto res = traverse(key, YAML::Clone(root)); !res) {
        return std::nullopt;
    } else {
        final = res.value();
    }

    try {
        return final.as<bool>();
    } catch (const YAML::Exception &) {
        return std::nullopt;
    }
}

std::optional<std::vector<std::string>> Configuration::get_string_vector(const std::string &key) const {
    YAML::Node final;
    if (auto res = traverse(key, YAML::Clone(root)); !res) {
        return std::nullopt;
    } else {
        final = res.value();
    }

    try {
        return final.as<std::vector<std::string>>();
    } catch (const YAML::Exception &) {
        return std::nullopt;
    }
}
