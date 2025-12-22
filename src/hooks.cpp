#include "catalyst/log-utils/log.hpp"
#include "catalyst/process_exec.h"
#include "catalyst/yaml-utils/Configuration.hpp"
#include "yaml-cpp/node/node.h"

#include <catalyst/hooks.hpp>
#include <cstdlib>
#include <expected>
#include <format>
#include <iostream>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace catalyst::hooks {

namespace {
std::expected<void, std::string> execute_hook(const YAML::Node &profile_comp, const std::string &hook_name) {
    catalyst::logger.log(LogLevel::DEBUG, "Executing hook: {}", hook_name);
    if (!profile_comp["hooks"] || !profile_comp["hooks"][hook_name]) {
        catalyst::logger.log(LogLevel::DEBUG, "No hook defined for: {}", hook_name);
        return {}; // No hook defined, so we do nothing.
    }

    auto shell_cmd = [](const std::string &cmd) -> std::vector<std::string> {
#if defined(_WIN32)
        return {"cmd", "/c", cmd};
#else
        return {"/bin/sh", "-c", cmd};
#endif
    };

    const auto &hook_node = profile_comp["hooks"][hook_name];
    if (hook_node.IsSequence()) {
        for (const auto &item : hook_node) {
            if (item["command"]) {
                std::string command = item["command"].as<std::string>();
                catalyst::logger.log(LogLevel::DEBUG, "[Catalyst Hook: {}] Running command: {}", hook_name, command);
                if (catalyst::process_exec(shell_cmd(command)).value().get() != 0) {
                    catalyst::logger.log(LogLevel::ERROR, "Hook '{}' command failed: {}", hook_name, command);
                    return std::unexpected("Hook '" + hook_name + "' command failed: " + command);
                }
            } else if (item["script"]) {
                std::string script = item["script"].as<std::string>();
                catalyst::logger.log(LogLevel::DEBUG, "[Catalyst Hook: {}] Running script: {}", hook_name, script);
                if (catalyst::process_exec(shell_cmd(script)).value().get() != 0) {
                    catalyst::logger.log(LogLevel::ERROR, "Hook '{}' script failed: {}", hook_name, script);
                    return std::unexpected("Hook '" + hook_name + "' script failed: " + script);
                }
            }
        }
    } else if (hook_node.IsScalar()) {
        std::string command = hook_node.as<std::string>();
        catalyst::logger.log(LogLevel::DEBUG, "[Catalyst Hook: {}] Running command: {}", hook_name, command);
        if (catalyst::process_exec(shell_cmd(command)).value().get() != 0) {
            catalyst::logger.log(LogLevel::ERROR, "Hook '{}' command failed: {}", hook_name, command);
            return std::unexpected("Hook '" + hook_name + "' command failed: " + command);
        }
    }

    catalyst::logger.log(LogLevel::DEBUG, "Hook finished successfully: {}", hook_name);
    return {};
}
} // namespace

std::expected<void, std::string> pre_build(const YAML::Node &profile_comp) {
    return execute_hook(profile_comp, "pre-build");
}

std::expected<void, std::string> post_build(const YAML::Node &profile_comp) {
    return execute_hook(profile_comp, "post-build");
}

std::expected<void, std::string> on_build_failure(const YAML::Node &profile_comp) {
    return execute_hook(profile_comp, "on-build-failure");
}

std::expected<void, std::string> pre_generate(const YAML::Node &profile_comp) {
    return execute_hook(profile_comp, "pre-generate");
}

std::expected<void, std::string> post_generate(const YAML::Node &profile_comp) {
    return execute_hook(profile_comp, "post-generate");
}

std::expected<void, std::string> pre_fetch(const YAML::Node &profile_comp) {
    return execute_hook(profile_comp, "pre-fetch");
}

std::expected<void, std::string> post_fetch(const YAML::Node &profile_comp) {
    return execute_hook(profile_comp, "post-fetch");
}

std::expected<void, std::string> pre_clean(const YAML::Node &profile_comp) {
    return execute_hook(profile_comp, "pre-clean");
}

std::expected<void, std::string> post_clean(const YAML::Node &profile_comp) {
    return execute_hook(profile_comp, "post-clean");
}

std::expected<void, std::string> pre_run(const YAML::Node &profile_comp) {
    return execute_hook(profile_comp, "pre-run");
}

std::expected<void, std::string> post_run(const YAML::Node &profile_comp) {
    return execute_hook(profile_comp, "post-run");
}

std::expected<void, std::string> pre_test(const YAML::Node &profile_comp) {
    return execute_hook(profile_comp, "pre-test");
}

std::expected<void, std::string> post_test(const YAML::Node &profile_comp) {
    return execute_hook(profile_comp, "post-test");
}

std::expected<void, std::string> pre_link(const YAML::Node &profile_comp) {
    return execute_hook(profile_comp, "pre-link");
}

std::expected<void, std::string> post_link(const YAML::Node &profile_comp) {
    return execute_hook(profile_comp, "post-link");
}

std::expected<void, std::string> pre_build(const YAML_UTILS::Configuration &profile_comp) {
    return execute_hook(profile_comp.get_root(), "pre-build");
}

std::expected<void, std::string> post_build(const YAML_UTILS::Configuration &profile_comp) {
    return execute_hook(profile_comp.get_root(), "post-build");
}

std::expected<void, std::string> on_build_failure(const YAML_UTILS::Configuration &profile_comp) {
    return execute_hook(profile_comp.get_root(), "on-build-failure");
}

std::expected<void, std::string> pre_generate(const YAML_UTILS::Configuration &profile_comp) {
    return execute_hook(profile_comp.get_root(), "pre-generate");
}

std::expected<void, std::string> post_generate(const YAML_UTILS::Configuration &profile_comp) {
    return execute_hook(profile_comp.get_root(), "post-generate");
}

std::expected<void, std::string> pre_fetch(const YAML_UTILS::Configuration &profile_comp) {
    return execute_hook(profile_comp.get_root(), "pre-fetch");
}

std::expected<void, std::string> post_fetch(const YAML_UTILS::Configuration &profile_comp) {
    return execute_hook(profile_comp.get_root(), "post-fetch");
}

std::expected<void, std::string> pre_clean(const YAML_UTILS::Configuration &profile_comp) {
    return execute_hook(profile_comp.get_root(), "pre-clean");
}

std::expected<void, std::string> post_clean(const YAML_UTILS::Configuration &profile_comp) {
    return execute_hook(profile_comp.get_root(), "post-clean");
}

std::expected<void, std::string> pre_run(const YAML_UTILS::Configuration &profile_comp) {
    return execute_hook(profile_comp.get_root(), "pre-run");
}

std::expected<void, std::string> post_run(const YAML_UTILS::Configuration &profile_comp) {
    return execute_hook(profile_comp.get_root(), "post-run");
}

std::expected<void, std::string> pre_test(const YAML_UTILS::Configuration &profile_comp) {
    return execute_hook(profile_comp.get_root(), "pre-test");
}

std::expected<void, std::string> post_test(const YAML_UTILS::Configuration &profile_comp) {
    return execute_hook(profile_comp.get_root(), "post-test");
}

std::expected<void, std::string> pre_link(const YAML_UTILS::Configuration &profile_comp) {
    return execute_hook(profile_comp.get_root(), "pre-link");
}

std::expected<void, std::string> post_link(const YAML_UTILS::Configuration &profile_comp) {
    return execute_hook(profile_comp.get_root(), "post-link");
}

std::expected<void, std::string> on_compile([[maybe_unused]] const std::filesystem::path &file) {
    // The on_compile hook would need a different implementation,
    // as it's not tied to the main profile composition.
    // For now, we'll leave it as a no-op.
    return {};
}

} // namespace catalyst::hooks
