#include "catalyst/subcommands/fetch/action.hpp"
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
std::expected<void, std::string> fetch_git(std::string build_dir, std::string name, std::string source,
                                           std::string version);


std::expected<void, std::string> action(const parse_t &parse_args) {
    if (std::find(parse_args.profiles.begin(), parse_args.profiles.end(), "common") == parse_args.profiles.end()) {
        const_cast<std::vector<std::string> &>(parse_args.profiles).insert(parse_args.profiles.begin(), "common");
    }
    std::string build_dir;
    if (auto res = fetch_profile("common"); res) {
        if (res.value()["manifest"] && res.value()["manifest"]["dirs"] && res.value()["manifest"]["dirs"]["build"]) {
            build_dir = res.value()["manifest"]["dirs"]["build"].as<std::string>();
        } else {
            return std::unexpected("manifest.dirs.build not found in common profile");
        }
    } else {
        return std::unexpected(res.error());
    }
    for (auto profile : parse_args.profiles) {
        std::expected<YAML::Node, std::string> res = fetch_profile(profile);
        if (!res)
            return std::unexpected(res.error());
        auto deps = res.value()["dependencies"];
        if (!deps)
            continue;
        if (!deps.IsSequence())
            return std::unexpected("dependencies field is not a sequence");
        for (auto dep : deps) {
            if (!dep["name"] || !dep["source"] || !dep["version"]) {
                return std::unexpected("Improperly configued dependency.");
            }
            std::string name = dep["name"].as<std::string>();
            std::string source = dep["source"].as<std::string>();
            std::string version = dep["version"].as<std::string>();
            if (source == "vcpkg") {
                if (auto res = fetch_vcpkg(name); !res)
                    return std::unexpected(res.error());
            } else {
                if (auto res = fetch_git(build_dir, name, source, version); !res)
                    return std::unexpected(res.error());
            }
        }
    }
    return {};
}
} // namespace catalyst::fetch
