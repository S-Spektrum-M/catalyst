#include <CLI/App.hpp>
#include <catalyst/subcommands/fetch/parse_cli.hpp>

namespace catalyst::fetch {
std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app) {
    CLI::App *fetch = app.add_subcommand("fetch", "fetch all dependencies for a profile");
    auto ret = std::make_unique<parse_t>();
    fetch->add_option("--profiles", ret->profiles);
    return {fetch, std::move(ret)};
}
}; // namespace catalyst::fetch
