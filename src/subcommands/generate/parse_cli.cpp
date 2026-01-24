#include "catalyst/subcommands/generate.hpp"

namespace catalyst::generate {

std::pair<CLI::App *, std::unique_ptr<Parse>> parse(CLI::App &app) {
    CLI::App *generate = app.add_subcommand("generate", "Generate a build file.");
    auto ret = std::make_unique<Parse>();
    generate->add_option("-p,--profiles", ret->profiles);
    generate->add_option("-f,--features", ret->enabled_features);
    return {generate, std::move(ret)};
}
} // namespace catalyst::generate
