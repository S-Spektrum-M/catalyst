#include "catalyst/GLOBALS.hpp"
#include "catalyst/subcommands/init/action.hpp"
#include "catalyst/yaml-utils/profile_write_back.hpp"
#include <catalyst/subcommands/init/parse_cli.hpp>
#include <expected>
#include <filesystem>
#include <fstream>
#include <string>
#include <yaml-cpp/node/node.h>

namespace catalyst::init {
std::expected<void, std::string> action(const parse_t &parse_args) {
    YAML::Node node;
    node["meta"]["min_ver"] = catalyst::CATALYST_VERSION;
    node["manifest"]["name"] = parse_args.name;
    switch (parse_args.type) {
    case parse_t::type_t::BINARY:
        node["manifest"]["type"] = "BINARY";
        break;
    case parse_t::type_t::STATICLIB:
        node["manifest"]["type"] = "STATICLIB";
        break;
    case parse_t::type_t::SHAREDLIB:
        node["manifest"]["type"] = "SHAREDLIB";
        break;
    case parse_t::type_t::HEADER:
        node["manifest"]["type"] = "HEADER";
        break;
    }
    node["manifest"]["version"] = parse_args.version;
    node["manifest"]["description"] = parse_args.description;
    node["manifest"]["provides"] = parse_args.provides;
    node["manifest"]["tooling"]["CC"] = parse_args.tooling.CC;
    node["manifest"]["tooling"]["CXX"] = parse_args.tooling.CXX;
    node["manifest"]["tooling"]["CCFLAGS"] = parse_args.tooling.CCFLAGS;
    node["manifest"]["tooling"]["CXXFLAGS"] = parse_args.tooling.CXXFLAGS;
    node["manifest"]["dirs"]["include"] = parse_args.dirs.include;
    node["manifest"]["dirs"]["source"] = parse_args.dirs.source;
    node["manifest"]["dirs"]["build"] = parse_args.dirs.build;

    namespace fs = std::filesystem;
    for (auto dir : parse_args.dirs.include) {
        if (!fs::exists(dir)) {
            fs::create_directory(dir);
        }
    }
    for (auto dir : parse_args.dirs.source) {
        if (!fs::exists(dir)) {
            fs::create_directory(dir);
        }
        auto ignore_path = fs::path(dir) / ".catalystignore";
        if (!fs::exists(ignore_path)) { std::ofstream{ignore_path}; }
    }
    if (!fs::exists(parse_args.dirs.build))
        fs::create_directory(parse_args.dirs.build);

    return catalyst::YAML_UTILS::profile_write_back(parse_args.profile, std::move(node));
}
} // namespace catalyst::init
