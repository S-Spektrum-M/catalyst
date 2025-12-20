#include "catalyst/subcommands/build.hpp"

#include <CLI/App.hpp>

namespace catalyst::build {
std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app) {
    CLI::App *build = app.add_subcommand("build", "build the project");
    auto ret = std::make_unique<parse_t>();
    build->add_flag("-r,--no-regen", ret->regen, "regenerate the build file");
    build->add_flag("--force-rebuild", ret->force_rebuild, "recompile dependencies");
    build->add_flag("--force-refetch", ret->force_refetch, "refetch dependencies");
    build->add_option("-p,--profiles", ret->profiles);
    build->add_option("-f,--features", ret->enabled_features);
    return {build, std::move(ret)};
}
} // namespace catalyst::build
