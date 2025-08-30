#include <catalyst/hooks.hpp>
#include <yaml-cpp/yaml.h>
#include <string>
#include <vector>
#include <iostream>
#include <expected>

namespace catalyst::hooks {

namespace {
std::expected<void, std::string> execute_hook(const YAML::Node &profile_comp, const std::string& hook_name) {
    if (!profile_comp["hooks"] || !profile_comp["hooks"][hook_name]) {
        return {}; // No hook defined, so we do nothing.
    }

    const auto& hook_node = profile_comp["hooks"][hook_name];
    if (hook_node.IsSequence()) {
        for (const auto& item : hook_node) {
            if (item["command"]) {
                std::string command = item["command"].as<std::string>();
                std::cout << "[Catalyst Hook: " << hook_name << "] Running command: " << command << std::endl;
                if (std::system(command.c_str()) != 0) {
                    return std::unexpected("Hook '" + hook_name + "' command failed: " + command);
                }
            } else if (item["script"]) {
                std::string script = item["script"].as<std::string>();
                std::cout << "[Catalyst Hook: " << hook_name << "] Running script: " << script << std::endl;
                if (std::system(script.c_str()) != 0) {
                    return std::unexpected("Hook '" + hook_name + "' script failed: " + script);
                }
            }
        }
    } else if (hook_node.IsScalar()) {
        std::string command = hook_node.as<std::string>();
        std::cout << "[Catalyst Hook: " << hook_name << "] Running command: " << command << std::endl;
        if (std::system(command.c_str()) != 0) {
            return std::unexpected("Hook '" + hook_name + "' command failed: " + command);
        }
    }

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

std::expected<void, std::string> on_compile([[maybe_unused]] const std::filesystem::path &file) {
    // The on_compile hook would need a different implementation,
    // as it's not tied to the main profile composition.
    // For now, we'll leave it as a no-op.
    return {};
}

} // namespace catalyst::hooks