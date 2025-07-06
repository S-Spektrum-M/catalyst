#include "catalyst/subcommands/generate/parse_cli.hpp"
#include <expected>
#include <filesystem>
#include <string>
#include <vector>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/node/type.h>

namespace catalyst::generate {

std::string ver_max(std::string s1, std::string s2) {
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
        if (v1[i] > v2[i])
            return s1;
        else if (v1[i] < v2[i])
            return s2;
        else
            continue;

    std::unreachable();
}

std::expected<YAML::Node, std::string> profile_comp(const std::vector<std::string> &profiles) {
    // define with defaults and add onto this
    YAML::Node ret_profile;
    ret_profile["meta"]["min_ver"] = "0.0.1";
    ret_profile["manifest"]["name"] = "name";
    ret_profile["manifest"]["type"] = "BINARY";
    ret_profile["manifest"]["version"] = "0.0.1";
    ret_profile["manifest"]["provides"] = "";
    ret_profile["manifest"]["tooling"]["CC"] = "clang";
    ret_profile["manifest"]["tooling"]["CXX"] = "clang++";
    ret_profile["manifest"]["tooling"]["CCFLAGS"] = "";
    ret_profile["manifest"]["tooling"]["CXXFLAGS"] = "";
    ret_profile["manifest"]["dirs"]["include"] = std::vector<std::string>{};
    ret_profile["manifest"]["dirs"]["source"] = std::vector<std::string>{};
    ret_profile["manifest"]["dirs"]["build"] = "";
    ret_profile["dependencies"] = std::vector<YAML::Node>{};
    ret_profile["features"] = std::vector<std::string>{};
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
        if (!fs::exists(path)) {
            return std::unexpected(std::format("Profile: {} not found", path.string()));
        }
        auto new_profile = YAML::LoadFile(path);
        if (new_profile["meta"]["min_ver"].IsDefined()) {
            ret_profile["meta"]["min_ver"] = ver_max(ret_profile["meta"]["min_ver"].as<std::string>(),
                                                     new_profile["meta"]["min_ver"].as<std::string>());
        }
        if (YAML::Node new_profile_manifest = new_profile["manifest"]; new_profile_manifest.IsDefined()) {
            if (new_profile_manifest["name"]) {
                ret_profile["manifest"]["name"] = new_profile_manifest["name"];
            }
            if (new_profile_manifest["type"]) {
                ret_profile["manifest"]["type"] = new_profile_manifest["type"];
            }
            if (new_profile_manifest["version"]) {
                ret_profile["manifest"]["version"] = new_profile_manifest["version"];
            }
            if (new_profile_manifest["provides"]) {
                ret_profile["manifest"]["provides"] = new_profile_manifest["provides"];
            }
            if (YAML::Node new_profile_manifest_tooling = new_profile_manifest["tooling"];
                new_profile_manifest_tooling) {
                if (new_profile_manifest_tooling["CC"]) {
                    ret_profile["manifest"]["tooling"]["CC"] = new_profile_manifest_tooling["CC"];
                }
                if (new_profile_manifest_tooling["CXX"]) {
                    ret_profile["manifest"]["tooling"]["CXX"] = new_profile_manifest_tooling["CXX"];
                }
                if (new_profile_manifest_tooling["CCFLAGS"]) {
                    ret_profile["manifest"]["tooling"]["CCFLAGS"] = new_profile_manifest_tooling["CCFLAGS"];
                }
                if (new_profile_manifest_tooling["CXXFLAGS"]) {
                    ret_profile["manifest"]["tooling"]["CXXFLAGS"] = new_profile_manifest_tooling["CXXFLAGS"];
                }
            }
            if (YAML::Node new_profile_manifest_dirs = new_profile_manifest["dirs"]; new_profile_manifest_dirs) {
                if (new_profile_manifest_dirs["include"] && new_profile_manifest_dirs["include"].IsSequence())
                    for (auto inc : new_profile_manifest_dirs["include"].as<std::vector<std::string>>())
                        ret_profile["manifest"]["dirs"]["include"].push_back(inc);
                if (new_profile_manifest_dirs["source"] && new_profile_manifest_dirs["source"].IsSequence())
                    for (auto src : new_profile_manifest_dirs["source"].as<std::vector<std::string>>())
                        ret_profile["manifest"]["dirs"]["source"].push_back(src);
                if (new_profile_manifest_dirs["build"]) {
                        ret_profile["manifest"]["dirs"]["build"] = new_profile_manifest_dirs["build"];
                }
            }
        }
        if (new_profile["features"] && new_profile["features"].IsSequence()) {
            for (const auto &feature : new_profile["features"].as<std::vector<std::string>>()) {
                ret_profile["features"].push_back(feature);
            }
        }
    }
    return ret_profile;
}

std::expected<void, std::string> action(const parse_t &parse_args) {
    // calculate the profile composition
    auto _ = profile_comp(parse_args.profiles);
    // write ninja script to {build_dir}/ninja.build
    return {};
}
} // namespace catalyst::build
