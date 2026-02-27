#pragma once
#include <expected>
#include <filesystem>
#include <string>

#include "catalyst/yaml-utils/Configuration.hpp"

#include "yaml-cpp/node/node.h"

namespace catalyst::hooks {
std::expected<void, std::string> pre_clean(const YAML::Node &profile_comp);
std::expected<void, std::string> post_clean(const YAML::Node &profile_comp);
std::expected<void, std::string> pre_run(const YAML::Node &profile_comp);
std::expected<void, std::string> post_run(const YAML::Node &profile_comp);
std::expected<void, std::string> pre_test(const YAML::Node &profile_comp);
std::expected<void, std::string> post_test(const YAML::Node &profile_comp);

std::expected<void, std::string> pre_build(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> post_build(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> on_build_failure(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> pre_generate(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> post_generate(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> pre_fetch(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> post_fetch(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> pre_clean(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> post_clean(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> pre_run(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> post_run(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> pre_test(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> post_test(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> pre_link(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> post_link(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> on_compile(const std::filesystem::path &file);
}; // namespace catalyst::hooks
