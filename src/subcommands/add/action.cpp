#include "catalyst/subcommands/add.hpp"
#include "catalyst/yaml-utils/load_profile_file.hpp"
#include "catalyst/yaml-utils/profile_write_back.hpp"
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/type.h>

namespace catalyst::add {
std::expected<void, std::string> action(const parse_t &parse_args) {
    for (const auto &profile_name : parse_args.profiles) {
        if (auto res = catalyst::YAML_UTILS::load_profile_file(profile_name); !res) {
            return std::unexpected(res.error());
        } else {
            YAML::Node &profile_node = res.value();
            bool dependency_found = false;
            if (profile_node["dependencies"]) {
                for (YAML::Node dependency : profile_node["dependencies"]) {
                    if (dependency["name"] && dependency["name"].as<std::string>() == parse_args.name) {
                        dependency["source"] = parse_args.source;
                        dependency["version"] = parse_args.version;
                        dependency["using"] = parse_args.enabled_features;
                        dependency_found = true;
                        break;
                    }
                }
            }
            if (!dependency_found) {
                YAML::Node newDependency;
                newDependency["name"] = parse_args.name;
                newDependency["source"] = parse_args.source;
                newDependency["version"] = parse_args.version;
                newDependency["using"] = parse_args.enabled_features;
                profile_node["dependencies"].push_back(newDependency);
            }
            if (auto write_res = catalyst::YAML_UTILS::profile_write_back(profile_name, std::move(profile_node));
                !write_res) {
                return std::unexpected(write_res.error());
            }
        }
    }
    return {};
}
} // namespace catalyst::add
