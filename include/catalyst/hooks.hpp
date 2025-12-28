#pragma once
#include "catalyst/yaml-utils/Configuration.hpp"
#include "yaml-cpp/node/node.h"

#include <expected>
#include <filesystem>
#include <string>

namespace catalyst::hooks {
std::expected<void, std::string> pre_build(const YAML::Node &profile_comp);
std::expected<void, std::string> post_build(const YAML::Node &profile_comp);
std::expected<void, std::string> on_build_failure(const YAML::Node &profile_comp);
std::expected<void, std::string> pre_generate(const YAML::Node &profile_comp);
std::expected<void, std::string> post_generate(const YAML::Node &profile_comp);
std::expected<void, std::string> pre_fetch(const YAML::Node &profile_comp);
std::expected<void, std::string> post_fetch(const YAML::Node &profile_comp);
std::expected<void, std::string> pre_clean(const YAML::Node &profile_comp);
std::expected<void, std::string> post_clean(const YAML::Node &profile_comp);
std::expected<void, std::string> pre_run(const YAML::Node &profile_comp);
std::expected<void, std::string> post_run(const YAML::Node &profile_comp);
std::expected<void, std::string> pre_test(const YAML::Node &profile_comp);
std::expected<void, std::string> post_test(const YAML::Node &profile_comp);
std::expected<void, std::string> pre_link(const YAML::Node &profile_comp);
std::expected<void, std::string> post_link(const YAML::Node &profile_comp);
std::expected<void, std::string> on_compile(const std::filesystem::path &file);

std::expected<void, std::string> pre_build(const YAML_UTILS::Configuration &profile_comp);
std::expected<void, std::string> post_build(const YAML_UTILS::Configuration &profile_comp);
std::expected<void, std::string> on_build_failure(const YAML_UTILS::Configuration &profile_comp);
std::expected<void, std::string> pre_generate(const YAML_UTILS::Configuration &profile_comp);
std::expected<void, std::string> post_generate(const YAML_UTILS::Configuration &profile_comp);
std::expected<void, std::string> pre_fetch(const YAML_UTILS::Configuration &profile_comp);
std::expected<void, std::string> post_fetch(const YAML_UTILS::Configuration &profile_comp);
std::expected<void, std::string> pre_clean(const YAML_UTILS::Configuration &profile_comp);
std::expected<void, std::string> post_clean(const YAML_UTILS::Configuration &profile_comp);
std::expected<void, std::string> pre_run(const YAML_UTILS::Configuration &profile_comp);
std::expected<void, std::string> post_run(const YAML_UTILS::Configuration &profile_comp);
std::expected<void, std::string> pre_test(const YAML_UTILS::Configuration &profile_comp);
std::expected<void, std::string> post_test(const YAML_UTILS::Configuration &profile_comp);
std::expected<void, std::string> pre_link(const YAML_UTILS::Configuration &profile_comp);
std::expected<void, std::string> post_link(const YAML_UTILS::Configuration &profile_comp);
std::expected<void, std::string> on_compile(const std::filesystem::path &file);
}; // namespace catalyst::hooks
