#include "catalyst/log-utils/log.hpp"
#include "catalyst/subcommands/add.hpp"
#include "catalyst/yaml-utils/load_profile_file.hpp"
#include "catalyst/yaml-utils/profile_write_back.hpp"
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/type.h>

namespace catalyst::add {
std::expected<void, std::string> action(const parse_t &parse_args) {
    catalyst::logger.log(catalyst::LogLevel::INFO, "Adding dependency: {}", parse_args.name);
    for (const auto &profile_name : parse_args.profiles) {
        if (auto res = catalyst::YAML_UTILS::load_profile_file(profile_name); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return std::unexpected(res.error());
        } else {
            YAML::Node &profile_node = res.value();
            bool dependency_found = false;
            if (profile_node["dependencies"]) {
                for (YAML::Node dependency : profile_node["dependencies"]) {
                    if (dependency["name"] && dependency["name"].as<std::string>() == parse_args.name) {
                        catalyst::logger.log(catalyst::LogLevel::INFO, "Updating dependency {}", parse_args.name);
                        dependency["source"] = parse_args.source;
                        dependency["version"] = parse_args.version;
                        dependency["using"] = parse_args.enabled_features;
                        dependency_found = true;
                        break;
                    }
                }
            }
            if (!dependency_found) {
                catalyst::logger.log(catalyst::LogLevel::INFO, "Creating new dependency {}", parse_args.name);
                YAML::Node newDependency;
                newDependency["name"] = parse_args.name;
                newDependency["source"] = parse_args.source;
                newDependency["version"] = parse_args.version;
                newDependency["using"] = parse_args.enabled_features;
                profile_node["dependencies"].push_back(newDependency);
            }
            if (auto write_res = catalyst::YAML_UTILS::profile_write_back(profile_name, std::move(profile_node));
                !write_res) {
                catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", write_res.error());
                return std::unexpected(write_res.error());
            }
            catalyst::logger.log(catalyst::LogLevel::INFO, "Dependency '{}' added to profile '{}'", parse_args.name,
                                 profile_name);
        }
    }
    return {};
}
} // namespace catalyst::add
