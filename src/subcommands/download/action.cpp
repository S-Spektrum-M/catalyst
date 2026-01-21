#include "catalyst/dir_gaurd.hpp"
#include "catalyst/log-utils/log.hpp"
#include "catalyst/process_exec.h"
#include "catalyst/subcommands/build.hpp"
#include "catalyst/subcommands/download.hpp"
#include "catalyst/subcommands/install.hpp"

#include <algorithm>
#include <filesystem>
#include <format>
#include <random>

namespace fs = std::filesystem;

namespace catalyst::download {

namespace {
std::string random_string(size_t length) {
    auto randchar = []() -> char {
        const char charset[] = "0123456789"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        static std::random_device rd;
        static std::mt19937 rng(rd());
        static std::uniform_int_distribution<size_t> dist(0, max_index - 1);
        return charset[dist(rng)];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}
} // namespace

std::expected<void, std::string> action(const parse_t &args) {
    catalyst::logger.log(LogLevel::DEBUG, "Download subcommand invoked.");

    auto absolute_target_path = fs::absolute(args.target_path);
    auto temp_dir = fs::temp_directory_path() / std::format("catalyst_dl_{}", random_string(8));

    // step 1: clone
    catalyst::logger.log(LogLevel::INFO, "Cloning {} into temporary directory {}", args.git_remote, temp_dir.string());

    std::vector<std::string> clone_cmd = {"git", "clone"};
    if (!args.git_branch.empty()) {
        clone_cmd.insert(clone_cmd.end(), {"--branch", args.git_branch});
    }
    clone_cmd.push_back(args.git_remote);
    clone_cmd.push_back(temp_dir.string());

    if (auto res = catalyst::process_exec(std::move(clone_cmd)); !res) {
        return std::unexpected(res.error());
    } else if (auto exit_code = res.value().get(); exit_code != 0) {
        return std::unexpected(std::format("Git clone failed with exit code {}", exit_code));
    }

    if (!fs::exists(temp_dir)) {
        return std::unexpected("Failed to create temporary directory for clone.");
    }

    catalyst::DirectoryChangeGuard scoped_dir(temp_dir);

    catalyst::logger.log(LogLevel::INFO,
                         "Building downloaded project with profiles: {} and features: {}",
                         std::format("{}", args.profiles),
                         std::format("{}", args.enabled_features));
    catalyst::build::parse_t build_args{
        .regen = false,
        .force_rebuild = false,
        .force_refetch = false,
        .workspace_build = false,
        .package = "",
        .profiles = args.profiles,
        .enabled_features = args.enabled_features,
        .workspace = std::nullopt,
    };

    if (auto res = catalyst::build::action(build_args); !res) {
        return std::unexpected(std::format("Build failed: {}", res.error()));
    }

    catalyst::logger.log(LogLevel::INFO, "Installing project to {}...", absolute_target_path.string());
    if (auto res = catalyst::install::action({
            .source_path = temp_dir,
            .target_path = absolute_target_path,
            .profiles = args.profiles,
        });
        !res) {
        return res;
    }

    catalyst::logger.log(LogLevel::INFO, "Successfully downloaded and installed {}.", args.git_remote);
    return {};
}
} // namespace catalyst::download
