#include "catalyst/log-utils/log.hpp"
#include "catalyst/process_exec.h"
#include "catalyst/subcommands/generate.hpp"

#include <catalyst/hooks.hpp>
#include <catalyst/subcommands/clean.hpp>
#include <format>
#include <string>
#include <yaml-cpp/node/node.h>

namespace catalyst::clean {
namespace fs = std::filesystem;

std::expected<void, std::string> action(const parse_t &parse_args) {
    catalyst::logger.log(LogLevel::DEBUG, "Clean subcommand invoked.");

    if (parse_args.workspace) {
        fs::path current = fs::current_path();
        bool is_root = false;
        try {
            is_root = fs::equivalent(parse_args.workspace->get_root(), current);
        } catch (...) {}

        if (is_root) {
            catalyst::logger.log(LogLevel::INFO, "Cleaning all workspace members.");
            bool any_failed = false;

            for (const auto& [name, member] : parse_args.workspace->get_members()) {
                 catalyst::logger.log(LogLevel::INFO, "Cleaning member: {}", name);
                 fs::current_path(member.path);
                 
                 parse_t member_args = parse_args;
                 member_args.workspace = std::nullopt; // Prevent recursion loop
                 
                 if (auto res = action(member_args); !res) {
                     catalyst::logger.log(LogLevel::ERROR, "Clean failed for member: {}", name);
                     any_failed = true;
                 }
                 fs::current_path(current);
            }
            if (any_failed) return std::unexpected("Clean failed for some members.");
            return {};
        }
    }

    auto profiles = parse_args.profiles;
    if (std::find(profiles.begin(), profiles.end(), "common") == profiles.end()) {
        profiles.insert(profiles.begin(), "common");
    }

    catalyst::logger.log(LogLevel::DEBUG, "Composing profiles.");
    YAML::Node profile_comp;
    if (auto res = generate::profile_composition(profiles); !res) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to compose profiles: {}", res.error());
        return std::unexpected(res.error());
    } else {
        profile_comp = res.value();
    }

    catalyst::logger.log(LogLevel::DEBUG, "Running pre-clean hooks.");
    if (auto res = hooks::pre_clean(profile_comp); !res) {
        catalyst::logger.log(LogLevel::ERROR, "Pre-clean hook failed: {}", res.error());
        return res;
    }

    std::string build_dir = profile_comp["manifest"]["dirs"]["build"].as<std::string>();
    std::string generator = profile_comp["meta"]["generator"].as<std::string>();
    catalyst::logger.log(LogLevel::DEBUG, "Cleaning build directory: {}", build_dir);
    if (generator == "ninja") {
        if (int rtn = catalyst::process_exec({"ninja", "-C", build_dir, "-t", "clean"}).value().get(); rtn != 0) {
            catalyst::logger.log(LogLevel::ERROR, "Failed to clean project.");
            return std::unexpected(
                std::format("Command: ninja -C {} -t clean failed with exit code: {}", build_dir, rtn));
        }
    } else if (generator == "cbe") {
        if (int rtn = catalyst::process_exec({"cbe", "-d", build_dir, "--clean"}).value().get(); rtn != 0) {
            catalyst::logger.log(LogLevel::ERROR, "Failed to clean project.");
            return std::unexpected(std::format("Command: cbe -d {} --clean failed with exit code: {}", build_dir, rtn));
        }
    }

    catalyst::logger.log(LogLevel::DEBUG, "Running post-clean hooks.");
    if (auto res = hooks::post_clean(profile_comp); !res) {
        catalyst::logger.log(LogLevel::ERROR, "Post-clean hook failed: {}", res.error());
        return res;
    }

    catalyst::logger.log(LogLevel::DEBUG, "Clean subcommand finished successfully.");
    return {};
}

std::expected<void, std::string> action2(const parse_t &parse_args) {
    catalyst::logger.log(LogLevel::DEBUG, "Clean subcommand invoked.");
    YAML_UTILS::Configuration config{parse_args.profiles};

    catalyst::logger.log(LogLevel::DEBUG, "Running pre-clean hooks.");
    if (auto res = hooks::pre_clean(config); !res) {
        catalyst::logger.log(LogLevel::ERROR, "Pre-clean hook failed: {}", res.error());
        return res;
    }

    std::string build_dir = config.get_string("manifest.dirs.build").value_or("build");
    catalyst::logger.log(LogLevel::DEBUG, "Cleaning build directory: {}", build_dir);
    if (catalyst::process_exec({"ninja", "-C", build_dir, "-t", "clean"}).value().get() != 0) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to clean project.");
        return std::unexpected("error in cleaning.");
    }

    catalyst::logger.log(LogLevel::DEBUG, "Running post-clean hooks.");
    if (auto res = hooks::post_clean(config); !res) {
        catalyst::logger.log(LogLevel::ERROR, "Post-clean hook failed: {}", res.error());
        return res;
    }

    catalyst::logger.log(LogLevel::DEBUG, "Clean subcommand finished successfully.");
    return {};
}
} // namespace catalyst::clean
