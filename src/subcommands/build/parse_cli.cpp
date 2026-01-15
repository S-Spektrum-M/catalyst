#include "catalyst/subcommands/build.hpp"

#include <CLI/App.hpp>
#include <vector>

namespace catalyst::build {
std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app) {
    CLI::App *build = app.add_subcommand("build", "Build the project.");
    auto ret = std::make_unique<parse_t>();
    build->add_flag("-r,--regen", ret->regen, "Regenerate the build file.")->default_val(false);
    build->add_flag("-b,--force-rebuild", ret->force_rebuild, "Recompile dependencies.")->default_val(false);
    build->add_flag("--force-refetch", ret->force_refetch, "Refetch dependencies.")->default_val(false);
    build->add_option("-p,--profiles", ret->profiles, "Profile composition to build.")
        ->default_val(std::vector{"common"});
    build->add_option("-f,--features", ret->enabled_features, "Features to enable.")
        ->default_val(std::vector<std::string>{});
    return {build, std::move(ret)};
}
} // namespace catalyst::build
