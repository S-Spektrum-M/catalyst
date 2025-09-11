#include "catalyst/yaml-utils/Configuration.hpp"
#include "catalyst/log-utils/log.hpp"
#include "yaml-cpp/node/node.h"
#include <algorithm>
#include <filesystem>
#include <iterator>
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
void merge(YAML::Node &target, const YAML::Node &source) {
    if (target.IsMap() && source.IsMap()) {
        for (auto it = source.begin(); it != source.end(); ++it) {
            const auto &key = it->first;
            const auto &value = it->second;

            bool key_found = false;
            for (auto tj = target.begin(); tj != target.end(); ++tj) {
                if (tj->first.as<std::string>() == key.as<std::string>()) {
                    merge(tj->second, value);
                    key_found = true;
                    break;
                }
            }

            if (!key_found) {
                target[key] = value;
            }
        }
    } else if (target.IsSequence() && source.IsSequence()) {
        for (const auto &item : source) {
            target.push_back(item);
        }
    } else {
        target = source;
    }
}

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

} // namespace

Configuration::Configuration(const std::vector<std::string> &profiles) {
    catalyst::logger.log(LogLevel::INFO, "Composing profiles.");
    root = getDefaultConfiguration();

    std::vector<std::string> profile_names = profiles;
    if (std::find(profile_names.begin(), profile_names.end(), "common") == profile_names.end()) {
        profile_names.insert(profile_names.cbegin(), "common");
    }

    std::vector<YAML::Node> loaded_profiles;
    for (const auto &profile_name : profile_names) {
        fs::path path{};
        if (profile_name == "common") {
            path = "catalyst.yaml";
        } else {
            path = std::format("catalyst_{}.yaml", profile_name);
        }

        if (fs::exists(path)) {
            catalyst::logger.log(LogLevel::INFO, "Loading profile: {}", path.string());
            try {
                loaded_profiles.push_back(YAML::LoadFile(path.string()));
            } catch (const YAML::Exception &e) {
                catalyst::logger.log(LogLevel::ERROR, "Failed to load profile {}: {}", path.string(), e.what());
            }
        } else {
            catalyst::logger.log(LogLevel::WARN, "Profile not found: {}", path.string());
        }
    }

    for (const auto &profile : loaded_profiles) {
        merge(root, profile);
    }

    std::string final_min_ver = "0.0.1";
    for (const auto &profile : loaded_profiles) {
        if (profile["meta"] && profile["meta"]["min_ver"]) {
            final_min_ver = ver_max(final_min_ver, profile["meta"]["min_ver"].as<std::string>());
        }
    }
    root["meta"]["min_ver"] = final_min_ver;

    catalyst::logger.log(LogLevel::INFO, "Profile composition finished.");
}

bool Configuration::has(const std::string &key) const {
    std::stringstream ss(key);
    std::string segment;
    std::vector<std::string> segments;
    while (std::getline(ss, segment, '.')) {
        segments.push_back(segment);
    }

    YAML::Node current = root;
    for (const auto &segment : segments) {
        if (!current[segment]) {
            return false;
        }
        current = current[segment];
    }
    return true;
}

std::optional<std::string> Configuration::get_string(const std::string &key) const {
    std::stringstream ss(key);
    std::string segment;
    std::vector<std::string> segments;
    while (std::getline(ss, segment, '.')) {
        segments.push_back(segment);
    }

    YAML::Node current = root;
    for (const auto &s : segments) {
        if (!current[s]) {
            return std::nullopt;
        }
        current = current[s];
    }

    try {
        return current.as<std::string>();
    } catch (const YAML::Exception &) {
        return std::nullopt;
    }
}

std::optional<int> Configuration::get_int(const std::string &key) const {
    std::stringstream ss(key);
    std::string segment;
    std::vector<std::string> segments;
    while (std::getline(ss, segment, '.')) {
        segments.push_back(segment);
    }

    YAML::Node current = root;
    for (const auto &s : segments) {
        if (!current[s]) {
            return std::nullopt;
        }
        current = current[s];
    }

    try {
        return current.as<int>();
    } catch (const YAML::Exception &) {
        return std::nullopt;
    }
}

std::optional<bool> Configuration::get_bool(const std::string &key) const {
    std::stringstream ss(key);
    std::string segment;
    std::vector<std::string> segments;
    while (std::getline(ss, segment, '.')) {
        segments.push_back(segment);
    }

    YAML::Node current = root;
    for (const auto &s : segments) {
        if (!current[s]) {
            return std::nullopt;
        }
        current = current[s];
    }

    try {
        return current.as<bool>();
    } catch (const YAML::Exception &) {
        return std::nullopt;
    }
}

std::optional<std::vector<std::string>>
Configuration::get_string_vector(const std::string &key) const {
    std::stringstream ss(key);
    std::string segment;
    std::vector<std::string> segments;
    while (std::getline(ss, segment, '.')) {
        segments.push_back(segment);
    }

    YAML::Node current = root;
    for (const auto &s : segments) {
        if (!current[s]) {
            return std::nullopt;
        }
        current = current[s];
    }

    if (current.IsSequence()) {
        try {
            return current.as<std::vector<std::string>>();
        } catch (const YAML::Exception &) {
            return std::nullopt;
        }
    }
    return std::nullopt;
}
