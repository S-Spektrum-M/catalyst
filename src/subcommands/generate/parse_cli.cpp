#include "catalyst/subcommands/generate/parse_cli.hpp"

namespace catalyst::generate {

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app) {
    CLI::App *build = app.add_subcommand("add", "add a dependency");
    auto ret = std::make_unique<parse_t>();
    build->add_option("-p,--profiles", ret->profiles);
    build->add_option("-f,--features", ret->enabled_features);
    return {build, std::move(ret)};
}
} // namespace catalyst::add
