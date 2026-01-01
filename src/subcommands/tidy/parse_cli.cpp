#include "catalyst/subcommands/tidy.hpp"

#include <CLI/App.hpp>
#include <vector>

namespace catalyst::tidy {

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app) {
    CLI::App *tidy = app.add_subcommand("tidy", "Run linting on source code.");
    auto ret = std::make_unique<parse_t>();
    tidy->add_option("-p,--profiles", ret->profiles, "The profile composition to lint")
        ->default_val(std::vector<std::string>{"common"});
    return {tidy, std::move(ret)};
}
} // namespace catalyst::tidy
