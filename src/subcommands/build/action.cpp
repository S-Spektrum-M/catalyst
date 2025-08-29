#include <algorithm>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <vector>
#include <yaml-cpp/node/node.h>

#include "catalyst/hooks.hpp"
#include "catalyst/subcommands/build.hpp"
#include "catalyst/subcommands/fetch.hpp"
#include "catalyst/subcommands/generate.hpp"

namespace catalyst::build {
namespace fs = std::filesystem;

bool dep_missing(const YAML::Node &pc) {
    fs::path build_dir = pc["manifest"]["dirs"]["build"].as<std::string>();
    if (!pc["dependencies"])
        return false; // program has no dependencies
    if (std::any_of(pc["dependencies"].begin(), pc["dependencies"].end(), [&](const YAML::Node &dep) {
            return !fs::exists(build_dir / "catalyst-libs" / dep["name"].as<std::string>());
        }))
        return true;
    return false;
}

// FIXME: use better redirection scheme
std::expected<void, std::string> generate_compile_commands(const fs::path &build_dir) {
    fs::path real_compdb_path = build_dir / "compile_commands.json";
    std::string compdb_command =
        std::format("ninja -C {} -t compdb > {}", build_dir.string(), real_compdb_path.string());
    if (std::system(compdb_command.c_str()) != 0) {
        return std::unexpected("failed to generate compile commands");
    }
    return {};
}

std::expected<void, std::string> action(const parse_t &parse_args) {
    std::vector<std::string> profiles = parse_args.profiles;
    if (std::find(profiles.begin(), profiles.end(), "common") == profiles.end()) {
        profiles.insert(profiles.begin(), "common");
    }

    YAML::Node profile_comp;
    if (auto res = generate::profile_composition(profiles); !res)
        return std::unexpected(res.error());
    else
        profile_comp = res.value();

    if (auto res = hooks::pre_build(profile_comp); !res) {
        if (auto hook_res = hooks::on_build_failure(profile_comp); !hook_res) {
            return std::unexpected(res.error() +
                                   "\nAdditionally, the on_build_failure hook failed with error: " + hook_res.error());
        }
        return res;
    }

    fs::path build_dir = profile_comp["manifest"]["dirs"]["build"].as<std::string>();
    bool regenerate = false;
    if (fs::exists(build_dir / "profile_composition.yaml")) {
        YAML::Node existing_pc = YAML::LoadFile((build_dir / "profile_composition.yaml").string());
        if (existing_pc.size() != profile_comp.size()) {
            regenerate = true;
        }
    }

    // DONE: generate the build file if requested or needed
    if (!fs::exists(build_dir / "build.ninja") || parse_args.regen || regenerate) {
        auto res = catalyst::generate::action({parse_args.profiles, parse_args.enabled_features});
        if (!res) {
            if (auto hook_res = hooks::on_build_failure(profile_comp); !hook_res) {
                return std::unexpected(
                    res.error() + "\nAdditionally, the on_build_failure hook failed with error: " + hook_res.error());
            }
            return std::unexpected(res.error());
        }
    }

    // TODO: check if all deps exist or we've been asked to refetch them
    if (!fs::exists(build_dir / "catalyst-libs") || parse_args.force_refetch || dep_missing(profile_comp)) {
        if (parse_args.force_refetch) {
            fs::remove_all(fs::path{build_dir / "catalyst-libs"}); // cleanup
        }
        if (auto res = catalyst::fetch::action({parse_args.profiles}); !res) {
            if (auto hook_res = hooks::on_build_failure(profile_comp); !hook_res) {
                return std::unexpected(
                    res.error() + "\nAdditionally, the on_build_failure hook failed with error: " + hook_res.error());
            }
            return std::unexpected(res.error());
        }
    }

    if (std::system(std::format("ninja -C {}", build_dir.string()).c_str()) != 0) {
        if (auto hook_res = hooks::on_build_failure(profile_comp); !hook_res) {
            return std::unexpected(
                "Failed to build project.\nAdditionally, the on_build_failure hook failed with error: " +
                hook_res.error());
        }
        return std::unexpected("Failed to build project");
    }

    // effectively: call "ninja -C build_dir -t compdb > build_dir/compile_commands.json"
    if (auto res = generate_compile_commands(build_dir); !res) {
        if (auto hook_res = hooks::on_build_failure(profile_comp); !hook_res) {
            return std::unexpected(res.error() +
                                   "\nAdditionally, the on_build_failure hook failed with error: " + hook_res.error());
        }
        return res;
    }

    if (auto res = hooks::post_build(profile_comp); !res) {
        if (auto hook_res = hooks::on_build_failure(profile_comp); !hook_res) {
            return std::unexpected(res.error() +
                                   "\nAdditionally, the on_build_failure hook failed with error: " + hook_res.error());
        }
        return res;
    }
    return {};
}
} // namespace catalyst::build
