#include "parse_rules.h"
#include "subcommand_actions.h"
#include "utils.h"
#include <filesystem>
#include <fstream>
#include <yaml-cpp/node/node.h>

void subcommand_init_action(init_args_t &args) {
    namespace fs = std::filesystem;
    if (fs::exists("catalyst.yaml") && !args.reinit) {
        log_print(log_level::ERROR, "catalyst.yaml already exists");
        log_print(log_level::INFO, "To reinitialize, rerun this command with '--reinit'");
        return;
    }
    YAML::Node root;
    {
        YAML::Node manifest;
        manifest["name"] = args.proj_name;
        manifest["version"] = args.proj_version;
        manifest["type"] = [](const init_args_t::ProjectType &pt) -> std::string {
            switch (pt) {
            case init_args_t::ProjectType::BINARY:
                return "BINARY";
            case init_args_t::ProjectType::STATIC:
                return "STATIC";
            case init_args_t::ProjectType::SHARED:
                return "SHARED";
            case init_args_t::ProjectType::HEADER:
                return "HEADER";
            }
        }(args.proj_type);
        manifest["description"] = "your description here";
        {
            YAML::Node tooling;
            tooling["CC"] = args.tooling.CC;
            tooling["CXX"] = args.tooling.CXX;
            tooling["CCFLAGS"] = args.tooling.CCFLAGS;
            tooling["CXXFLAGS"] = args.tooling.CXXFLAGS;
            manifest["tooling"] = tooling;
        }
        {
            YAML::Node dirs;
            dirs["include"] = args.include_dirs;
            dirs["source"] = args.src_dirs;
            dirs["build"] = args.build_dir;
            manifest["dirs"] = dirs;
        }
        root["manifest"] = manifest;
    }
    std::ofstream cayaml{"catalyst.yaml"};
    cayaml << root << std::endl;
}
