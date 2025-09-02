#include "catalyst/subcommands/init.hpp"
#include <CLI/App.hpp>
#include <map>

namespace catalyst::init {
std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app) {
    CLI::App *init = app.add_subcommand("init", "initialize a profile.");
    auto ret = std::make_unique<parse_t>();
    init->add_option("-n,--name", ret->name, "the name of the project");
    init->add_option("--path", ret->path, "the default path for the project");

    std::map<std::string, parse_t::type_t> type_map{{"binary", parse_t::type_t::BINARY},
                                                    {"staticlib", parse_t::type_t::STATICLIB},
                                                    {"sharedlib", parse_t::type_t::SHAREDLIB},
                                                    {"interface", parse_t::type_t::INTERFACE}};
    init->add_option("-t,--type", ret->type, "the project type")
        ->transform(CLI::CheckedTransformer(type_map, CLI::ignore_case));

    init->add_option("-v,--version", ret->version, "the project's version");

    init->add_option("-d,--description", ret->description, "a description for the project");
    init->add_option("--provides", ret->provides, "a list of provided libraries for this project.");

    init->add_option("--cc", ret->tooling.CC, "the c compiler to use");
    init->add_option("--cxx", ret->tooling.CXX, "the cxx compiler to use");
    init->add_option("--ccflags", ret->tooling.CCFLAGS, "c compiler flags");
    init->add_option("--cxxflags", ret->tooling.CXXFLAGS, "cxx compiler flags");

    init->add_option("--include-dirs", ret->dirs.include, "include directories");
    init->add_option("--source-dirs", ret->dirs.source, "source directories");
    init->add_option("--build-dir", ret->dirs.build, "build directory");

    init->add_option("-p,--profile", ret->profile, "the profile to initialize");

    return {init, std::move(ret)};
}
}; // namespace catalyst::init
