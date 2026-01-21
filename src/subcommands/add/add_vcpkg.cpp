#include "catalyst/log-utils/log.hpp"
#include "catalyst/yaml-utils/load_profile_file.hpp"
#include "catalyst/yaml-utils/profile_write_back.hpp"
#include "yaml-cpp/node/node.h"

#include <catalyst/subcommands/add.hpp>
#include <expected>
#include <string>
#include <vector>

static inline std::expected<void, std::string> add_to_profile(const std::string &profile,
                                                              const catalyst::add::vcpkg::parse_t &args);

namespace catalyst::add::vcpkg {
std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &add) {
    CLI::App *add_vcpkg = add.add_subcommand("vcpkg", "add a vcpkg dependency");
    auto ret = std::make_unique<parse_t>();

    add_vcpkg->add_option("name", ret->name)->required();
    add_vcpkg->add_option("-t,--triplet", ret->triplet)->required();
    add_vcpkg->add_option("-v,--version", ret->version)->default_str("latest");
    add_vcpkg->add_option("-p,--profiles", ret->profiles)->default_val(std::vector<std::string>{"common"});
    add_vcpkg->add_option("-f,--features", ret->enabled_features);

    return {add_vcpkg, std::move(ret)};
}

std::expected<void, std::string> action(const parse_t &parse_args) {
    for (const auto &profile_name : parse_args.profiles) {
        if (auto res = add_to_profile(profile_name, parse_args); !res)
            return std::unexpected(res.error());
    }
    return {};
}

} // namespace catalyst::add::vcpkg

static inline std::expected<void, std::string> add_to_profile(const std::string &profile,
                                                              const catalyst::add::vcpkg::parse_t &args) {
    auto res = catalyst::YAML_UTILS::load_profile_file(profile);
    if (!res) {
        catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
        return std::unexpected(res.error());
    }
    YAML::Node profile_node = res.value();

    if (!profile_node["dependencies"]) {
        profile_node["dependencies"] = YAML::Node(YAML::NodeType::Sequence);
    }

    YAML::Node dependencies = profile_node["dependencies"];

    bool dependency_found = false;
    for (const auto &dep : dependencies) {
        if (dep["name"] && dep["name"].as<std::string>() == args.name) {
            dependency_found = true;
            break;
        }
    }

    if (dependency_found) {
        return std::unexpected("Dependency '" + args.name + "' already exists in profile '" + profile + "'.");
    }

    YAML::Node new_dep;
    new_dep["name"] = args.name;
    new_dep["source"] = "vcpkg";
    if (!args.triplet.empty()) {
        new_dep["triplet"] = args.triplet;
    }
    if (!args.version.empty()) {
        new_dep["version"] = args.version;
    } else {
        new_dep["version"] = "latest";
    }
    if (!args.enabled_features.empty()) {
        new_dep["using"] = args.enabled_features;
    }

    dependencies.push_back(new_dep);

    return catalyst::YAML_UTILS::profile_write_back(profile, std::move(profile_node));
}
