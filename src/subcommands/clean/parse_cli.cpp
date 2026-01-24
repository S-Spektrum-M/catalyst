#include "catalyst/subcommands/clean.hpp"

namespace catalyst::clean {
std::pair<CLI::App *, std::unique_ptr<Parse>> parse(CLI::App &app) {
    CLI::App *clean = app.add_subcommand("clean", "Clean artifacts.");
    auto ret = std::make_unique<Parse>();
    clean->add_option("-p,--profile", ret->profiles);
    return {clean, std::move(ret)};
}
} // namespace catalyst::clean
