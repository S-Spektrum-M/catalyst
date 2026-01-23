#include "catalyst/log-utils/log.hpp"
#include "catalyst/subcommands/configure.hpp"
#include "yaml-cpp/node/node.h"

#include <catalyst/yaml-utils/load_profile_file.hpp>
#include <catalyst/yaml-utils/profile_write_back.hpp>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace catalyst::configure {

bool is_int(const std::string &str);

std::expected<void, std::string> action(const parse_t &parse_args) {
    catalyst::logger.log(LogLevel::DEBUG, "Configure subcommand invoked.");
    // 1. Parse the variable string
    size_t colon_pos = parse_args.var.find(':');
    std::string profile;
    std::string var_path_str;
    if (colon_pos == std::string::npos) {
        profile = "common";
        var_path_str = parse_args.var;
    } else {
        profile = parse_args.var.substr(0, colon_pos);
        var_path_str = parse_args.var.substr(colon_pos + 1);
    }
    catalyst::logger.log(LogLevel::DEBUG, "Configuring variable '{}' in profile '{}'", var_path_str, profile);

    // 2. Load the profile
    catalyst::logger.log(LogLevel::DEBUG, "Loading profile file for '{}'", profile);
    YAML::Node profile_node;
    if (auto res = YAML_UTILS::load_profile_file(profile); !res) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to load profile: {}", res.error());
        return std::unexpected(res.error());
    } else {
        profile_node = res.value();
    }

    // 3. Figure out the var path
    std::vector<std::string> var_path;
    std::string_view var_path_sv(var_path_str);
    for (auto it = var_path_sv.begin(); it != var_path_sv.end();) {
        auto next_dot = std::find(it, var_path_sv.end(), '.');
        var_path.emplace_back(it, next_dot);
        it = next_dot;
        if (it != var_path_sv.end()) {
            it++;
        }
    }

    YAML::Node target_node = profile_node;
    // 5. Update the value
    if (!parse_args.val.empty()) {
        catalyst::logger.log(LogLevel::DEBUG, "Updating value to '{}'", parse_args.val);
        for (size_t i = 0; i < var_path.size() - 1; ++i) {
            target_node = target_node[var_path[i]];
        }
        if (is_int(parse_args.val)) {
            target_node[var_path.back()] = std::stoi(parse_args.val);
        } else {
            target_node[var_path.back()] = parse_args.val;
        }
        auto res = YAML_UTILS::profileWriteBack(profile, profile_node);
        if (!res) {
            catalyst::logger.log(LogLevel::ERROR, "Failed to write back profile: {}", res.error());
        }
        return res;
    }
    catalyst::logger.log(LogLevel::DEBUG, "No value provided, printing existing value.");
    for (size_t i = 0; i < var_path.size(); ++i) {
        target_node = target_node[var_path[i]];
    }
    std::cout << target_node << std::endl;
    catalyst::logger.log(LogLevel::DEBUG, "Configure subcommand finished successfully.");
    return {};
}

bool is_int(const std::string &str) {
    if (str.empty())
        return false;

    for (size_t i = 0; i < str.length(); ++i)
        if (!isdigit(str[i]))
            return false;

    return true;
}
} // namespace catalyst::configure
