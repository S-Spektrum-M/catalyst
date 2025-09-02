#include "catalyst/hooks.hpp"
#include "catalyst/subcommands/fetch.hpp"
#include "catalyst/subcommands/generate.hpp"
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
std::expected<void, std::string> fetch_local(std::string build_dir, std::string name, std::string source,
                                             std::string version);
std::expected<void, std::string> fetch_git(std::string build_dir, std::string name, std::string source,
                                           std::string version);
std::expected<void, std::string> fetch_system(const std::string &name);

std::expected<void, std::string> action(const parse_t &parse_args) {
    auto profiles = parse_args.profiles;
    if (std::find(profiles.begin(), profiles.end(), "common") == profiles.end()) {
        profiles.insert(profiles.begin(), "common");
    }

    YAML::Node profile_comp;
    if (auto res = generate::profile_composition(profiles); !res) {
        return std::unexpected(res.error());
    } else {
        profile_comp = res.value();
    }

    if (auto res = hooks::pre_fetch(profile_comp); !res) {
        return res;
    }

    std::string build_dir = profile_comp["manifest"]["dirs"]["build"].as<std::string>();

    auto deps = profile_comp["dependencies"];
    if (deps && deps.IsSequence()) {
        for (auto dep : deps) {
            if (!dep["name"] || !dep["source"]) {
                return std::unexpected("Improperly configured dependency.");
            }
            std::string name = dep["name"].as<std::string>();
            std::string source = dep["source"].as<std::string>();
            if (source == "vcpkg") {
                if (!dep["version"]) {
                    return std::unexpected(std::format("vcpkg dependency '{}' is missing version.", name));
                }
                if (auto res = fetch_vcpkg(name); !res)
                    return std::unexpected(res.error());
            } else if (source == "system") {
                if (auto res = fetch_system(name); !res)
                    return std::unexpected(res.error());
            } else if (source == "local") {
                std::println(std::cout, "Skipping fetch for local dependency: {}", name);
            } else {
                if (!dep["version"]) {
                    return std::unexpected(std::format("git dependency '{}' is missing version.", name));
                }
                std::string version = dep["version"].as<std::string>();
                if (auto res = fetch_git(build_dir, name, source, version); !res)
                    return std::unexpected(res.error());
            }
        }
    }

    if (auto res = hooks::post_fetch(profile_comp); !res) {
        return res;
    }

    return {};
}
} // namespace catalyst::fetch
