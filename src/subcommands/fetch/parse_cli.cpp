#include "catalyst/subcommands/fetch.hpp"

#include <CLI/App.hpp>

namespace catalyst::fetch {
std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app) {
    CLI::App *fetch = app.add_subcommand("fetch", "Fetch all dependencies for a profile composition.");
    auto ret = std::make_unique<parse_t>();
    fetch->add_option("--profiles", ret->profiles);
    return {fetch, std::move(ret)};
}
}; // namespace catalyst::fetch
