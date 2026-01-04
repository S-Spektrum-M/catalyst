#include "catalyst/subcommands/init.hpp"

#include <CLI/App.hpp>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace catalyst::init {
std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app) {
    CLI::App *init = app.add_subcommand("init", "Initialize a new catalyst profile.");
    auto ret = std::make_unique<parse_t>();
    init->add_option("-n,--name", ret->name, "the name of the project")
        ->default_str(std::filesystem::current_path().filename().string());
    init->add_option("--path", ret->path, "the default path for the project")
        ->default_val(std::filesystem::current_path());

    std::map<std::string, parse_t::type_t> type_map{{"binary", parse_t::type_t::BINARY},
                                                    {"staticlib", parse_t::type_t::STATICLIB},
                                                    {"sharedlib", parse_t::type_t::SHAREDLIB},
                                                    {"interface", parse_t::type_t::INTERFACE}};
    init->add_option("-t,--type", ret->type, "the project type")
        ->transform(CLI::CheckedTransformer(type_map, CLI::ignore_case))
        ->default_val(parse_t::type_t::BINARY);

    init->add_option("-v,--version", ret->version, "the project's version")->default_str("0.0.1");

    init->add_option("-d,--description", ret->description, "a description for the project")->default_str("Your Description Goes Here.");
    init->add_option("--provides", ret->provides, "Artifact provided by this project.")->default_str("");

    init->add_option("--cc", ret->tooling.CC, "the c compiler to use")->default_str("clang");
    init->add_option("--cxx", ret->tooling.CXX, "the cxx compiler to use")->default_str("clang++");
    init->add_option("--ccflags", ret->tooling.CCFLAGS, "c compiler flags")->default_str("");
    init->add_option("--cxxflags", ret->tooling.CXXFLAGS, "cxx compiler flags")->default_str("");

    init->add_option("--include-dirs", ret->dirs.include, "include directories")->default_val(std::vector<std::string>{"include"});
    init->add_option("--source-dirs", ret->dirs.source, "source directories")->default_val(std::vector<std::string>{"src"});
    init->add_option("--build-dir", ret->dirs.build, "build directory")->default_str("build");

    init->add_option("-p,--profile", ret->profile, "the profile to initialize")->default_val("common");

    return {init, std::move(ret)};
}
} // namespace catalyst::init
