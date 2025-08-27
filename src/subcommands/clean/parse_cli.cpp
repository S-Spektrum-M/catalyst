#include "catalyst/subcommands/clean.hpp"

namespace catalyst::clean {
    std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app) {
        CLI::App *clean = app.add_subcommand("clean", "clean a dependency");
        auto ret = std::make_unique<parse_t>();
        clean->add_option("-p,--profile", ret->profiles);
        return {clean, std::move(ret)};
    }
} // namespace catalyst::clean
