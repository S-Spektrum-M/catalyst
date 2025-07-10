#include "catalyst/subcommands/fetch/action.hpp"
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h>

namespace catalyst::fetch {
namespace fs = std::filesystem;
std::expected<YAML::Node, std::string> fetch_profile(const std::string profile_name) {
    fs::path profile_path;
    if (profile_name == "common")
        profile_path = "catalyst.yaml";
    else
        profile_path = std::format("catalyst_{}.yaml", profile_name);
    if (!fs::exists(profile_path)) {
        return std::unexpected(
            std::format("configuration: {} for profile: {} does not exist", profile_path.string(), profile_name));
    }
    return YAML::LoadFile(profile_path);
}

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
        if (auto res = fetch_profile(profile); !res) {
            return std::unexpected(res.error());
        } else {
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
                std::string command;
                fs::path dep_path = fs::path(build_dir) / "catalyst-libs" / name;
                if (version == "latest") {
                    command = std::format("git clone --depth 1 {} {}", source, dep_path.string());
                } else {
                    command = std::format("git clone --depth 1 --branch {} {} {}", version, source, dep_path.string());
                }
                std::cout << std::format("Fetching: {}@{} from {}\n", name, version, source);
                if (std::system(command.c_str()) != 0) {
                    return std::unexpected(std::format("Failed to fetch dependency: {}", name));
                }
                std::cout << std::format("Building: {}\n", name);
                // NOTE: after build command is impelmented, we can just do "catalyst build <PATH>"
                command = std::format("cd {} && mkdir build && catalyst generate && ninja -C build", dep_path.string());
                if (std::system(command.c_str()) != 0) {
                    return std::unexpected(std::format("Failed to build dependency: {}", name));
                }
            }
        }
    }
    return {};
}
} // namespace catalyst::fetch
