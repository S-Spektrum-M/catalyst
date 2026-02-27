#pragma once
#include <expected>
#include <filesystem>
#include <string>

#include "catalyst/yaml_utils/configuration.hpp"

#include "yaml-cpp/node/node.h"

namespace catalyst::hooks {
std::expected<void, std::string> preClean(const YAML::Node &profile_comp);
std::expected<void, std::string> postClean(const YAML::Node &profile_comp);
std::expected<void, std::string> preRun(const YAML::Node &profile_comp);
std::expected<void, std::string> postRun(const YAML::Node &profile_comp);
std::expected<void, std::string> preTest(const YAML::Node &profile_comp);
std::expected<void, std::string> postTest(const YAML::Node &profile_comp);

std::expected<void, std::string> preBuild(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> postBuild(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> onBuildFailure(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> preGenerate(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> postGenerate(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> preFetch(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> postFetch(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> preClean(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> postClean(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> preRun(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> postRun(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> preTest(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> postTest(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> preLink(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> postLink(const yaml_utils::Configuration &profile_comp);
std::expected<void, std::string> onCompile(const std::filesystem::path &file);
}; // namespace catalyst::hooks
