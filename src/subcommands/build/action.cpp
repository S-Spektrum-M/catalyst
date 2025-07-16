#include <array>
#include <cstdio>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <vector>
#include <yaml-cpp/node/node.h>

#include "catalyst/subcommands/build/parse_cli.hpp"
#include "catalyst/subcommands/fetch/action.hpp"
#include "catalyst/subcommands/generate/action.hpp"
#include "catalyst/subcommands/generate/parse_cli.hpp"

namespace catalyst::build {
namespace fs = std::filesystem;
std::expected<void, std::string> action(const parse_t &parse_args) {
    // TODO: check if the exsiting build file matches the provided profile profile composition std::vector<
    std::vector<std::string> profiles = parse_args.profiles;
    if (std::find(profiles.begin(), profiles.end(), "common") == profiles.end()) {
        profiles.insert(profiles.begin(), "common");
    }
    YAML::Node pc;
    if (auto res = generate::profile_composition(profiles); !res) {
        return std::unexpected(res.error());
    } else {
        pc = res.value();
    }
    std::string build_dir_str = pc["manifest"]["dirs"]["build"].as<std::string>();
    bool regenerate = false;
    if (fs::exists(std::format("{}/{}", build_dir_str, "profile_composition.yaml"))) {
        YAML::Node existing_pc = YAML::LoadFile(std::format("{}/{}", build_dir_str, "profile_composition.yaml"));
        if (existing_pc.size() != pc.size()) {
            regenerate = true;
        }
    }

    // DONE: generate the build file if requested or needed
    if (!fs::exists(std::format("{}/{}", build_dir_str, "build.ninja")) || parse_args.regen || regenerate) {
        auto res = catalyst::generate::action({parse_args.profiles, parse_args.enabled_features});
        if (!res)
            return std::unexpected(res.error());
    }

    // TODO: check if all deps exist or we've been asked to refetch them
    if (!fs::exists(std::format("{}/{}", build_dir_str, "catalyst-libs")) || parse_args.force_refetch) {
        auto res = catalyst::fetch::action({parse_args.profiles});
        if (!res)
            return std::unexpected(res.error());
    }
    // TODO: check if all deps exist or we've been asked to refetch them

    // DONE: call "ninja -C build_dir_str"
    std::system(std::format("ninja -C {}", build_dir_str).c_str());

    // DONE: call "ninja -C build_dir_str -t compdb > build_dir_str/compile_commands.json"
    std::string compdb_command = std::format("ninja -C {} -t compdb", build_dir_str);
    FILE *pipe = popen(compdb_command.c_str(), "r");
    if (!pipe) {
        return std::unexpected("failed to output compile commands");
    }

    std::array<char, 256> buffer;
    std::string result;
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);

    std::string compdb_path = std::format("{}/compile_commands.json", build_dir_str);
    std::ofstream compdb_file(compdb_path);
    if (!compdb_file) {
        return std::unexpected(std::format("Failed to open {} for writing.", compdb_path));
    }
    compdb_file << result;

    return {};
}
} // namespace catalyst::build
