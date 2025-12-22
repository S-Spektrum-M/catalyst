#include "catalyst/log-utils/log.hpp"
#include "catalyst/process_exec.h"
#include "catalyst/subcommands/generate.hpp"

#include <catalyst/hooks.hpp>
#include <catalyst/subcommands/clean.hpp>
#include <format>
#include <yaml-cpp/node/node.h>

namespace catalyst::clean {
std::expected<void, std::string> action(const parse_t &parse_args) {
    catalyst::logger.log(LogLevel::DEBUG, "Clean subcommand invoked.");
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
    catalyst::logger.log(LogLevel::DEBUG, "Cleaning build directory: {}", build_dir);
    if (catalyst::process_exec({"ninja", "-C", build_dir, "-t", "clean"}).value().get() != 0) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to clean project.");
        return std::unexpected("error in cleaning.");
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
