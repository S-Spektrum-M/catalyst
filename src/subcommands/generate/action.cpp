#include "catalyst/subcommands/generate/parse_cli.hpp"
#include <expected>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <regex>
#include <string>
#include <unordered_set>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace catalyst::generate {

std::string ver_max(std::string s1, std::string s2) {
    auto split_ver = [](const std::string &ver) {
        std::vector<int> parts;
        std::istringstream iss(ver);
        for (std::string token; std::getline(iss, token, '.');) {
            try {
                parts.push_back(std::stoi(token));
            } catch (...) {
                parts.push_back(0);
            }
        }
        while (parts.size() < 3)
            parts.push_back(0);
        return parts;
    };

    auto v1 = split_ver(s1);
    auto v2 = split_ver(s2);

    for (size_t i = 0, lim = std::min(v1.size(), v2.size()); i < lim; ++i)
        if (v1[i] > v2[i])
            return s1;
        else if (v1[i] < v2[i])
            return s2;
        else
            continue;

    return s1;
}

std::expected<YAML::Node, std::string> profile_composition(const std::vector<std::string> &profiles) {
    // define with defaults and add onto this
    YAML::Node composite;
    composite["meta"]["min_ver"] = "0.0.1";
    composite["manifest"]["name"] = "name";
    composite["manifest"]["type"] = "BINARY";
    composite["manifest"]["version"] = "0.0.1";
    composite["manifest"]["provides"] = "";
    composite["manifest"]["tooling"]["CC"] = "clang";
    composite["manifest"]["tooling"]["CXX"] = "clang++";
    composite["manifest"]["tooling"]["CCFLAGS"] = "";
    composite["manifest"]["tooling"]["CXXFLAGS"] = "";
    composite["manifest"]["dirs"]["include"] = std::vector<std::string>{};
    composite["manifest"]["dirs"]["source"] = std::vector<std::string>{};
    composite["manifest"]["dirs"]["build"] = "";
    composite["dependencies"] = std::vector<YAML::Node>{};
    composite["features"] = std::vector<std::string>{};
    namespace fs = std::filesystem;
    std::vector<fs::path> profile_paths;
    for (const std::string &profile_name : profiles) {
        fs::path path{};
        if (profile_name == "common")
            profile_paths.push_back("catalyst.yaml");
        else
            profile_paths.push_back(std::format("catalyst_{}.yaml", profile_name));
    }
    for (const auto &path : profile_paths) {
        if (!fs::exists(path)) {
            return std::unexpected(std::format("Profile: {} not found", path.string()));
        }
        auto new_profile = YAML::LoadFile(path);
        if (new_profile["meta"]["min_ver"].IsDefined()) {
            composite["meta"]["min_ver"] = ver_max(composite["meta"]["min_ver"].as<std::string>(),
                                                   new_profile["meta"]["min_ver"].as<std::string>());
        }
        if (YAML::Node new_profile_manifest = new_profile["manifest"]; new_profile_manifest.IsDefined()) {
            if (new_profile_manifest["name"]) {
                composite["manifest"]["name"] = new_profile_manifest["name"];
            }
            if (new_profile_manifest["type"]) {
                composite["manifest"]["type"] = new_profile_manifest["type"];
            }
            if (new_profile_manifest["version"]) {
                composite["manifest"]["version"] = new_profile_manifest["version"];
            }
            if (new_profile_manifest["provides"]) {
                composite["manifest"]["provides"] = new_profile_manifest["provides"];
            }
            if (YAML::Node new_profile_manifest_tooling = new_profile_manifest["tooling"];
                new_profile_manifest_tooling) {
                if (new_profile_manifest_tooling["CC"]) {
                    composite["manifest"]["tooling"]["CC"] = new_profile_manifest_tooling["CC"];
                }
                if (new_profile_manifest_tooling["CXX"]) {
                    composite["manifest"]["tooling"]["CXX"] = new_profile_manifest_tooling["CXX"];
                }
                if (new_profile_manifest_tooling["CCFLAGS"]) {
                    composite["manifest"]["tooling"]["CCFLAGS"] = new_profile_manifest_tooling["CCFLAGS"];
                }
                if (new_profile_manifest_tooling["CXXFLAGS"]) {
                    composite["manifest"]["tooling"]["CXXFLAGS"] = new_profile_manifest_tooling["CXXFLAGS"];
                }
            }
            if (YAML::Node new_profile_manifest_dirs = new_profile_manifest["dirs"]; new_profile_manifest_dirs) {
                if (new_profile_manifest_dirs["include"] && new_profile_manifest_dirs["include"].IsSequence())
                    for (auto inc : new_profile_manifest_dirs["include"].as<std::vector<std::string>>())
                        composite["manifest"]["dirs"]["include"].push_back(inc);
                if (new_profile_manifest_dirs["source"] && new_profile_manifest_dirs["source"].IsSequence())
                    for (auto src : new_profile_manifest_dirs["source"].as<std::vector<std::string>>())
                        composite["manifest"]["dirs"]["source"].push_back(src);
                if (new_profile_manifest_dirs["build"]) {
                    composite["manifest"]["dirs"]["build"] = new_profile_manifest_dirs["build"];
                }
            }
        }
        if (new_profile["features"] && new_profile["features"].IsSequence()) {
            for (const auto &feature : new_profile["features"]) {
                composite["features"].push_back(feature.as<std::string>());
            }
        }
        if (new_profile["dependencies"] && new_profile["dependencies"].IsSequence()) {
            for (const YAML::Node &dep : new_profile["dependencies"]) {
                composite["dependencies"].push_back(dep);
            }
        }
    }
    return composite;
}

namespace fs = std::filesystem;

std::expected<std::unordered_set<fs::path>, std::string> build_source_set(std::vector<std::string> source_dirs,
                                                                          const std::vector<std::string> &profiles) {
    namespace sv = std::views;
    auto str_to_path = [](const std::string &str) { return fs::path{str}; };
    auto paths = source_dirs | sv::transform(str_to_path) | std::ranges::to<std::vector>();
    std::unordered_set<fs::path> ret_set;
    std::unordered_set<std::string> ignored_files;

    for (const auto &dir : paths) {
        if (!fs::exists(dir) || !fs::is_directory(dir)) {
            return std::unexpected(std::format("Source directory not found or is not a directory: {}", dir.string()));
        }

        fs::path ignore_file = dir / ".catalystignore";
        if (fs::exists(ignore_file)) {
            YAML::Node ignore_config = YAML::LoadFile(ignore_file.string());
            for (const auto &profile : profiles) {
                if (ignore_config[profile]) {
                    for (const auto &ignore_pattern : ignore_config[profile]) {
                        ignored_files.insert(ignore_pattern.as<std::string>());
                    }
                }
            }
        }

        for (const auto &entry : fs::directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                bool ignored = false;
                for (const auto &pattern : ignored_files) {
                    std::regex ignore_regex(pattern);
                    if (std::regex_match(entry.path().filename().string(), ignore_regex)) {
                        ignored = true;
                        break;
                    }
                }
                if (!ignored) {
                    const auto &path = entry.path();
                    const std::string extension = path.extension().string();
                    if (extension == ".cpp" || extension == ".cxx" || extension == ".cc" || extension == ".c") {
                        ret_set.insert(path);
                    }
                }
            }
        }
    }
    return ret_set;
}

void write_variables(const YAML::Node &profile, std::ofstream &buildfile) {
    fs::path current_dir = fs::current_path();
    std::string build_dir_str = profile["manifest"]["dirs"]["build"].as<std::string>();
    fs::path build_dir(build_dir_str);
    fs::path obj_dir = "obj";

    std::string cxxflags = profile["manifest"]["tooling"]["CXXFLAGS"].as<std::string>();
    std::string cflags = profile["manifest"]["tooling"]["CCFLAGS"].as<std::string>();
    if (auto includes = profile["manifest"]["dirs"]["include"]; includes && includes.IsSequence()) {
        for (const auto &dir : includes.as<std::vector<std::string>>()) {
            cxxflags += " -I" + (current_dir / dir).string();
            cflags += " -I" + (current_dir / dir).string();
        }
    }
    buildfile << "# Variables\n"
              << "cc = " << profile["manifest"]["tooling"]["CC"].as<std::string>() << "\n"
              << "cxx = " << profile["manifest"]["tooling"]["CXX"].as<std::string>() << "\n"
              << "cxxflags = " << cxxflags << "\n"
              << "cflags = " << cflags << "\n"
              << "builddir = " << build_dir.string() << "\n"
              << "objdir = " << obj_dir.string() << "\n\n";
}

void write_rules(std::ofstream &buildfile) {
    buildfile << "# Rules for compiling\n";
    buildfile << "rule cxx_compile\n";
    buildfile << "  command = $cxx $cxxflags -MD -MF $out.d -c $in -o $out\n";
    buildfile << "  description = CXX $out\n";
    buildfile << "  depfile = $out.d\n";
    buildfile << "  deps = gcc\n\n";

    buildfile << "rule c_compile\n";
    buildfile << "  command = $cc $cflags -MD -MF $out.d -c $in -o $out\n";
    buildfile << "  description = CC $out\n";
    buildfile << "  depfile = $out.d\n";
    buildfile << "  deps = gcc\n\n";

    // TODO: DISCOVER DEPS for a -L flag
    buildfile << "# Rules for linking\n";
    buildfile << "rule binary_link\n";
    buildfile << "  command = $cxx $in -o $out\n";
    buildfile << "  description = LINK $out\n\n";

    buildfile << "rule static_link\n";
    buildfile << "  command = ar rcs $out $in\n";
    buildfile << "  description = LINK $out\n\n";

    buildfile << "rule shared_link\n";
    buildfile << "  command = $cxx -shared $in -o $out\n";
    buildfile << "  description = LINK $out\n\n";
}

std::expected<void, std::string> action(const parse_t &parse_args) {
    std::vector<std::string> profiles = parse_args.profiles;
    // prevent people form neglecting COMMON but also allow ALT_CONFIG -> COMMON -> ALT_CONFIG2 typoe of profile comp.
    if (std::find(profiles.begin(), profiles.end(), "common") == profiles.end())
        profiles.insert(profiles.cbegin(), std::string{"common"});
    auto pc = profile_composition(profiles);
    if (!pc)
        return std::unexpected(pc.error());
    YAML::Node profile = pc.value();
    fs::path current_dir = fs::current_path();
    std::vector<std::string> relative_source_dirs =
        profile["manifest"]["dirs"]["source"].as<std::vector<std::string>>();
    std::vector<std::string> absolute_source_dirs;
    for (const auto &dir : relative_source_dirs) {
        absolute_source_dirs.push_back((current_dir / dir).string());
    }
    auto source_set_res = build_source_set(absolute_source_dirs, profiles);
    if (!source_set_res)
        return std::unexpected(source_set_res.error());

    const auto &source_set = source_set_res.value();

    std::string build_dir_str = profile["manifest"]["dirs"]["build"].as<std::string>();
    fs::path build_dir(build_dir_str);
    fs::path obj_dir = "obj";

    std::error_code ec;
    fs::create_directories(obj_dir, ec);
    if (ec) {
        return std::unexpected("Failed to create object directory: " + obj_dir.string() + " - " + ec.message());
    }

    std::ofstream buildfile{build_dir / "build.ninja"};
    if (!buildfile) {
        return std::unexpected("Failed to open build.ninja for writing in " + build_dir.string());
    }

    buildfile << "# Ninja build file generated by Catalyst\n\n";

    write_variables(profile, buildfile);
    write_rules(buildfile);

    // Build edges for source files
    buildfile << "# Source File Compilation\n";
    std::vector<std::string> object_files;
    for (const auto &src : source_set) {
        object_files.push_back(obj_dir / src.filename().replace_extension(".o"));
        buildfile << "build " << object_files.back() << ": "
                  << ((src.extension() == ".c") ? "c_compile" : "cxx_compile") << " " << src.string() << "\n";
    }
    buildfile << "\n";

    // Build edge for the final target
    std::string type = profile["manifest"]["type"].as<std::string>();
    std::string target_prefix;
    std::string target_suffix;
    std::string link_rule;

    if (type == "STATIC") {
        link_rule = "static_link";
#if defined(_WIN32)
        target_prefix = "";
        target_suffix = ".lib";
#else
        target_prefix = "lib";
        target_suffix = ".a";
#endif
    } else if (type == "SHARED") {
        link_rule = "shared_link";
#if defined(_WIN32)
        target_prefix = "";
        target_suffix = ".dll";
#elif defined(__APPLE__)
        target_prefix = "lib";
        target_suffix = ".dylib";
#else // Linux and other Unix-likes
        target_prefix = "lib";
        target_suffix = ".so";
#endif
    } else { // BINARY or default
        link_rule = "binary_link";
#if defined(_WIN32)
        target_suffix = ".exe";
#endif
    }

    std::string target_name = profile["manifest"]["name"].as<std::string>();
    fs::path target_path{target_prefix + target_name + target_suffix};
    buildfile << "# Build edge for the final target\n";
    buildfile << "build " << target_path.string() << ": " << link_rule;
    for (const auto &obj : object_files) {
        buildfile << " " << obj;
    }
    buildfile << "\n\n";

    // Default target
    buildfile << "# Default target to build\n";
    buildfile << "default " << target_path.string() << "\n";

    return {};
}
} // namespace catalyst::generate
