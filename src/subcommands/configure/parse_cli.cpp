#include <CLI/CLI.hpp>
#include <catalyst/subcommands/configure/parse_cli.hpp>

namespace catalyst::configure {
std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app) {
    CLI::App *configure = app.add_subcommand("configure", "set a configuration variable");
    auto ret = std::make_unique<parse_t>();
    configure->add_option("variable", ret->var, "The configuration variable to set.")->required();
    configure->add_option("value", ret->val, "The value to assign to the variable.")->required();
    return {configure, std::move(ret)};
}
}
