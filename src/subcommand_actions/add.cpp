#include "parse_rules.h"
#include "subcommand_actions.h"
#include "utils.h"
#include <filesystem>
#include <fstream>
#include <ranges>
#include <string>
#include <unistd.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>

std::string escape_special_chars(const std::string &input) {
    std::string escaped_string;
    // Pre-allocating memory can be a performance optimization for very long
    // strings to reduce the number of reallocations.
    escaped_string.reserve(input.length());

    for (char c : input) {
        if (c == '@' || c == ':') {
            escaped_string += '\\'; // Add the escape character
        }
        escaped_string += c; // Add the original character
    }
    return escaped_string;
}

std::string fix_dep_pkg(add_args_t &add_args) {
    add_args.dep_name = escape_special_chars(add_args.dep_name);
    if (add_args.dep_name != "") {
        if (add_args.dep_ver != "") {
            return std::format("{}:{}@{}", add_args.dep_src, add_args.dep_name, add_args.dep_ver);
        }
        return std::format("{}:{}", add_args.dep_src, add_args.dep_name, add_args.dep_ver);
    }
    return std::format("{}@{}", add_args.dep_name, add_args.dep_ver);
    return std::format("{}", add_args.dep_name);
}

void subcommand_add_action(add_args_t &add_args) {
    for (std::string platform : add_args.dep_platforms) {
        namespace fs = std::filesystem;
        namespace Y = YAML;
        fs::path profile_path;
        if (platform == "CATALYST_COMMON")
            profile_path = "catalyst.yaml";
        else
            profile_path = std::format("catalyst_{}.yaml", platform);
        Y::Node manifest;
        if (!fs::exists(profile_path)) {
            log_print(log_level::WARN, "Creating new profile specification: {} for platform: {}.",
                      profile_path.string(), platform);
        } else {
            try {
                manifest = Y::LoadFile(profile_path);
            } catch (const Y::Exception &e) {
                log_print(log_level::ERROR, "Failed to parse catalyst.yaml: {}", std::string(e.what()));
                return;
            }
        }

        // construct dependency
        manifest["dependencies"][add_args.dep_name]["version"] = add_args.dep_ver;
        manifest["dependencies"][add_args.dep_name]["source"] = add_args.dep_src;
        manifest["dependencies"][add_args.dep_name]["using"] =
            add_args.dep_feats | std::views::filter([](auto dep_feat) { return dep_feat != "CATALYST_COMMON"; }) |
            std::ranges::to<std::vector<std::string>>();
        // std::vector<std::string>{}; // empty list
        //                             // TODO: add support for adding with
        //                             // features

        // write back
        std::ofstream c{profile_path};
        c << manifest << std::endl;
        log_print(log_level::INFO, "Successfully added {} v.{} from {} to {}", add_args.dep_name, add_args.dep_ver,
                  add_args.dep_src, profile_path.string());
    }
}
