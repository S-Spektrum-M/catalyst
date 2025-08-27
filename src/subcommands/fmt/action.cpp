#include "catalyst/subcommands/fmt.hpp"
#include "catalyst/subcommands/generate.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace catalyst::fmt {
std::expected<void, std::string> action(const parse_t &parse_args) {
    const std::vector<std::string> &profiles = parse_args.profiles;
    YAML::Node profile_comp;
    if (auto res = generate::profile_composition(profiles); !res) {
        return std::unexpected(res.error());
    } else {
        profile_comp = res.value();
    }

    std::string formatter = profile_comp["manifest"]["tooling"]["FMT"].as<std::string>();

    namespace fs = std::filesystem;

    std::unordered_set<fs::path> source_dirs, include_dirs;

    for (const auto &node : profile_comp["manifest"]["dirs"]["source"]) {
        source_dirs.insert(node.as<std::string>());
    }

    for (const auto &node : profile_comp["manifest"]["dirs"]["include"]) {
        include_dirs.insert(node.as<std::string>());
    }

    std::vector<std::filesystem::path> files_to_format;
    for (const auto &dir : source_dirs) {
        for (const auto &entry : std::filesystem::recursive_directory_iterator(dir)) {
            if (entry.is_regular_file() && (entry.path().extension() == ".cpp" || entry.path().extension() == ".hpp")) {
                files_to_format.push_back(entry.path());
            }
        }
    }

    for (const auto &dir : include_dirs) {
        for (const auto &entry : std::filesystem::recursive_directory_iterator(dir)) {
            if (entry.is_regular_file() && (entry.path().extension() == ".cpp" || entry.path().extension() == ".hpp")) {
                files_to_format.push_back(entry.path());
            }
        }
    }

    for (const auto &file : files_to_format) {
        std::string command = formatter + " -i " + file.string();
        int res = std::system(command.c_str());
        if (res != 0) {
            return std::unexpected("Error running clang-format on " + file.string());
        }
    }

    return {};
}
} // namespace catalyst::fmt
