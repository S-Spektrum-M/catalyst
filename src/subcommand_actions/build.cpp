#include "parse_rules.h"
#include "subcommand_actions.h"
#include "utils.h"
#include <algorithm>
#include <complex>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <ranges>
#include <shared_mutex>
#include <string>
#include <unordered_set>
#include <vector>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>

namespace fs = std::filesystem;
// currently no support for args
void build_bin(const build_args_t &args, const fs::path &project_path);
void build_static();
void build_shared();
// don't know what this means but ok
void build_header();

void subcommand_build_action([[maybe_unused]] build_args_t &args) {
    fs::path project_path{"catalyst.yaml"}; // allow for ovrrides through --profile
    if (!fs::exists(project_path)) {
        log_print(log_level::ERROR, "{} does not exist.", project_path.string());
        return;
    }
    YAML::Node proj_info = YAML::LoadFile(project_path);
    if (!proj_info["manifest"].IsDefined()) {
        log_print(log_level::ERROR, "{} is missing field 'manifest'", project_path.string());
        return;
    }
    if (!proj_info["manifest"]["type"].IsDefined()) {
        log_print(log_level::ERROR, "{} manifest is missing field 'type'", project_path.string());
        return;
    }
    init_args_t::ProjectType type = init_args_t::type_map[proj_info["manifest"]["type"].as<std::string>()];
    switch (type) {
    case init_args_t::ProjectType::BINARY:
        build_bin(args, project_path);
        return;
    case init_args_t::ProjectType::STATIC:
        build_static();
        return;
    case init_args_t::ProjectType::SHARED:
        build_shared();
        return;
    case init_args_t::ProjectType::HEADER:

        build_header();
        return;
    }
}

/// @brief perform error checks and return string on success.
std::expected<YAML::Node, std::string> get_proj(const fs::path &project_path) {
    YAML::Node proj = YAML::LoadFile(project_path);
    if (!proj["manifest"].IsDefined())
        return std::unexpected("field 'manifest' not specified in " + project_path.string());
    if (!proj["manifest"]["dirs"].IsDefined())
        return std::unexpected("field 'manifest.dirs' not specified in " + project_path.string());
    if (!proj["manifest"]["dirs"]["source"].IsDefined())
        return std::unexpected("field 'manifest.dirs.source' not specified in " + project_path.string());
    if (!proj["manifest"]["dirs"]["source"].IsSequence())
        return std::unexpected("field 'manifest.dirs.source' is not defined as a yaml sequence in " +
                               project_path.string());
    return proj;
}

std::expected<std::unordered_set<fs::path>, std::string> build_source_set(YAML::Node &proj) {
    std::unordered_set<fs::path> res;
    std::vector<std::string> src_dirs = proj["manifest"]["dirs"]["source"].as<std::vector<std::string>>();
    for (const std::string &src_dir : src_dirs) {
        fs::path src_dir_path{src_dir};
        if (!fs::exists(src_dir_path)) {
            return std::unexpected(std::format("Unknown path: {} specified in catalyst.yaml", src_dir_path.string()));
        }

        std::unordered_set<std::string> ignore_list;
        fs::path ignore_file = src_dir_path / ".catalystignore";
        if (fs::exists(ignore_file)) {
            std::ifstream ignore_stream(ignore_file);
            std::string line;
            while (std::getline(ignore_stream, line)) {
                if (!line.empty()) {
                    ignore_list.insert(line);
                }
            }
        }
        // Iterate over the files in the source directory and add valid files not in ignore_list
        for (const auto &entry : fs::directory_iterator(src_dir_path)) {
            if (entry.is_regular_file() && (entry.path().extension() == ".c" || entry.path().extension() == ".cpp" ||
                                            entry.path().extension() == ".cc")) {
                if (ignore_list.find(entry.path().filename().string()) == ignore_list.end()) {
                    res.insert(entry.path());
                }
            }
        }
    }
    return res;
}

std::expected<std::string, std::string> get_dep_libs(YAML::Node &proj) {
    auto deps = proj["dependencies"];
    if (!deps.IsDefined() || !deps.IsMap())
        return std::unexpected("Failed to parse dependencies field.");

    std::string ret{""};
    for (const auto &dep : deps) {
        fs::path dep_manifest_path;
        if (!dep.second["profile"].IsDefined() || dep.second["profile"].as<std::string>() == "" ||
            dep.second["profile"].as<std::string>() == "common")
            dep_manifest_path = fs::path{"catalyst-libs"} / dep.first.as<std::string>() / "catalyst.yaml";
        else
            dep_manifest_path = fs::path{"catalyst-libs"} / dep.first.as<std::string>() /
                                std::format("catalyst_{}.yaml", dep.second["profile"].as<std::string>());

        if (!fs::exists(dep_manifest_path))
            return std::unexpected(std::format("Dependency: {} does not have appropriate catalyst manifest file: {}",
                                               dep.first.as<std::string>(), dep_manifest_path.string()));

        YAML::Node dep_profile = YAML::LoadFile(dep_manifest_path);
        if (dep_profile["manifest"]["provides"].IsDefined() && dep_profile["manifest"]["provides"].IsScalar()) {
            ret += std::format("{}/{} ", dep_profile["manifest"]["dirs"]["build"].as<std::string>(),
                               dep_profile["manifest"]["manifest"]["profile"].as<std::string>());
        } else {
            return std::unexpected(std::format("Dependency profile: {} does not define field: 'manifest.provides'",
                                               dep_manifest_path.string()));
        }
    }
    return ret;
}

void build_bin(const build_args_t &args, const fs::path &project_path) {
    fs::path profile_path;
    if (args.profile.empty())
        profile_path = "common.cbuild";
    else
        profile_path = args.profile + ".cbuild";
    if (fs::exists(profile_path))
        log_print(log_level::WARN, "'{}' already exists and may be overwritten.", profile_path.string());

    YAML::Node proj;
    if (auto res = get_proj(project_path); res)
        proj = res.value();
    else {
        log_print(log_level::ERROR, "{}", res.error());
        return;
    }

    std::unordered_set<fs::path> source_file_set;

    if (auto res = build_source_set(proj); res)
        source_file_set = res.value();
    else {
        log_print(log_level::ERROR, "{}", res.error());
        return;
    }
    std::ofstream build_file{profile_path};

    std::string feat_str;
    if ((!proj["features"].IsDefined() || !proj["features"].IsSequence()) && !args.features.empty()) {
        log_print(log_level::ERROR, "'features' field is improperly configured in profile specification: {}",
                  project_path.string());
        return;
    }
    std::set<std::string> all_features{};
    if (proj["features"].IsDefined() && proj["features"].IsSequence()) {
        all_features = proj["features"].as<std::vector<std::string>>() | std::ranges::to<std::set<std::string>>();
    }
    auto enabled_features = args.features | std::ranges::to<std::set<std::string>>();
    for (auto a : all_features) {
        feat_str += std::format("-D{}=", a);
        if (enabled_features.find(a) == enabled_features.end())
            feat_str += std::format("false ");
        else
            feat_str += std::format("true ");
    }
    feat_str = feat_str.substr(0, feat_str.size() - 1);

    std::string CC{"clang"}, CXX{"clang++"}, CCFLAGS{""}, CXXFLAGS{""};
    if (proj["manifest"]["tooling"]["CC"].IsDefined()) {
        CC = proj["manifest"]["tooling"]["CC"].as<std::string>();
    } else {
        log_print(log_level::WARN, "field: 'tooling.CC' was not defined, using default value: '{}'", CC);
    }
    if (proj["manifest"]["tooling"]["CXX"].IsDefined()) {
        CXX = proj["manifest"]["tooling"]["CXX"].as<std::string>();
    } else {
        log_print(log_level::WARN, "field: 'tooling.CXX' was not defined, using default value: '{}'", CXX);
    }
    if (proj["manifest"]["tooling"]["CCFLAGS"].IsDefined()) {
        CCFLAGS = proj["manifest"]["tooling"]["CCFLAGS"].as<std::string>();
    } else {
        log_print(log_level::WARN, "field: 'tooling.CCFLAGS' was not defined, using default value: '{}'", CCFLAGS);
    }
    if (proj["manifest"]["tooling"]["CXXFLAGS"].IsDefined()) {
        CXXFLAGS = proj["manifest"]["tooling"]["CXXFLAGS"].as<std::string>();
    } else {
        log_print(log_level::WARN, "field: 'tooling.CXXFLAGS' was not defined, using default value: '{}'", CXXFLAGS);
    }

    std::string inc_str{""};
    if (proj["manifest"]["dirs"]["include"].IsDefined() && proj["manifest"]["dirs"]["include"].IsSequence()) {
        std::vector<std::string> vec_inc = proj["manifest"]["dirs"]["include"].as<std::vector<std::string>>();
        for (auto inc : vec_inc) {
            inc_str += "-I" + inc + " ";
        }
        inc_str = inc_str.substr(0, inc_str.size() - 1);
    }

    std::string cc_comp = CC + " -c " + CCFLAGS + feat_str + " " + inc_str;
    std::string cxx_comp = CXX + " -c " + CXXFLAGS + feat_str + " " + inc_str;
    std::println(build_file, "RULE CC_COMPILE: {} -c {} {} {} $in -o $out", CC, CCFLAGS, feat_str, inc_str);
    std::println(build_file, "RULE CXX_COMPILE: {} -c {} {} {} $in -o $out", CXX, CXXFLAGS, feat_str, inc_str);
    std::println(build_file, "RULE LINK: $in -o $out");
    auto clean_obj = [&proj](const std::string &in) {
        std::string ret = proj["manifest"]["dirs"]["build"].IsDefined()
                              ? proj["manifest"]["dirs"]["build"].as<std::string>()
                              : "build";
        ret += "/";
        for (auto ch : in) {
            if (ch == '/')
                ret += "_";
            else
                ret += ch;
        }
        ret += ".o";
        return ret;
    };
    namespace sv = std::views;
    namespace sr = std::ranges;
    using src_obj_pair = std::pair<fs::path, std::string>;
    auto make_src_obj_pair = [&clean_obj](fs::path f) -> src_obj_pair { return {f, clean_obj(f.string())}; };
    std::vector<src_obj_pair> src_obj_pairs =
        source_file_set | sv::transform(make_src_obj_pair) | sr::to<std::vector>();
    std::string objects{""};
    for (auto &[src, obj] : src_obj_pairs) {
        std::string compiler = [](const fs::path &f) {
            if (f.extension() == fs::path{".c"})
                return "CC_COMPILE";
            if (f.extension() == fs::path{".cpp"} || f.extension() == fs::path{".cc"})
                return "CXX_COMPILE";
            return "";
        }(src);
        std::println(build_file, "{} = {}({})", obj, compiler, src.string());
        objects += obj + " ";
    }

    if (auto res = get_dep_libs(proj); !res) {
        log_print(log_level::ERROR, "{}", res.error());
        return;
    }
    objects += get_dep_libs(proj).value();
    objects = objects.substr(0, objects.size() - 1);

    std::println(build_file, "{} = LINK({})", proj["manifest"]["name"].as<std::string>(), objects);
}

void build_static() {
    if (fs::exists("catalyst.build"))
        log_print(log_level::WARN, "overwritting existing catalyst.build.");
    std::ofstream build_file{"catalyst.build"};
}

void build_shared() {
    if (fs::exists("catalyst.build"))
        log_print(log_level::WARN, "overwritting existing catalyst.build.");
    std::ofstream build_file{"catalyst.build"};
}

// don't know what this means but ok
void build_header() {
    if (fs::exists("catalyst.build"))
        log_print(log_level::WARN, "overwritting existing catalyst.build.");
    std::ofstream build_file{"catalyst.build"};
}
