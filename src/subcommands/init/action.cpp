#include "catalyst/GLOBALS.hpp"
#include "catalyst/log-utils/log.hpp"
#include "catalyst/subcommands/init.hpp"

#include "catalyst/yaml-utils/profile_write_back.hpp"

#include <expected>
#include <filesystem>
#include <fstream>
#include <string>
#include <yaml-cpp/node/node.h>

namespace catalyst::init {

namespace fs = std::filesystem;

std::expected<void, std::string> action(const parse_t &parse_args) {
    catalyst::logger.log(LogLevel::DEBUG, "Init subcommand invoked.");
    // TODO: change directory to parse_args->path;
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
    case parse_t::type_t::INTERFACE:
        node["manifest"]["type"] = "INTERFACE";
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

    for (auto dir : parse_args.dirs.include) {
        if (!fs::exists(parse_args.path / dir)) {
            catalyst::logger.log(LogLevel::INFO, "Creating include directory: {}", (parse_args.path / dir).string());
            fs::create_directories(parse_args.path / dir);
        }
    }

    for (auto dir : parse_args.dirs.source) {
        if (!fs::exists(parse_args.path / dir)) {
            catalyst::logger.log(LogLevel::INFO, "Creating source directory: {}", (parse_args.path / dir).string());
            fs::create_directories(parse_args.path / dir);
        }
        auto ignore_path = fs::path(dir) / ".catalystignore";
        if (!fs::exists(ignore_path)) {
            catalyst::logger.log(LogLevel::INFO, "Creating .catalystignore file in: {}", dir);
            std::ofstream{ignore_path};
        }
    }

    if (!fs::exists(parse_args.path / parse_args.dirs.build)) {
        catalyst::logger.log(LogLevel::INFO, "Creating build directory: {}",
                             (parse_args.path / parse_args.dirs.build).string());
        fs::create_directories(parse_args.path / parse_args.dirs.build);
    }

    fs::path profile_path;
    if (parse_args.profile == "common")
        profile_path = std::format("{}/catalyst.yaml", parse_args.path.string());
    else
        profile_path = std::format("{}/catalyst_{}.yaml", parse_args.path.string(), parse_args.profile);

    if (!fs::exists(profile_path)) {
        catalyst::logger.log(LogLevel::DEBUG, "Creating new profile file: {}", profile_path.string());
    }

    std::ofstream profile_file = std::ofstream(profile_path);
    YAML::Emitter emmiter;
    emmiter << node;
    profile_file << emmiter.c_str() << std::endl;
    catalyst::logger.log(LogLevel::DEBUG, "Init subcommand finished successfully.");
    return {};
}

} // namespace catalyst::init
