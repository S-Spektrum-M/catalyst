#include "catalyst/subcommands/add/action.hpp"
#include "catalyst/yaml-utils/load_profile_file.hpp"
#include "catalyst/yaml-utils/profile_write_back.hpp"
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/type.h>

namespace catalyst::add {
std::expected<void, std::string> action(const parse_t &parse_args) {
    for (auto profile_name : parse_args.profiles) {
        if (auto res = catalyst::YAML_UTILS::load_profile_file(profile_name); !res) {
            return std::unexpected(res.error());
        } else {
            YAML::Node &profile_node = res.value();
            YAML::Node newDependency;
            newDependency["name"] = parse_args.name;
            newDependency["source"] = parse_args.source;
            newDependency["version"] = parse_args.version;
            newDependency["using"] = parse_args.enabled_features;
            profile_node["dependencies"].push_back(newDependency);
            if (auto res = catalyst::YAML_UTILS::profile_write_back(profile_name, std::move(profile_node)); !res) {
                return std::unexpected(res.error());
            }
        }
    }
    return {};
}
} // namespace catalyst::add
