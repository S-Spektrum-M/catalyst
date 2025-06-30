#include "catalyst/subcommands/add/action.hpp"
#include "catalyst/subcommands/add/parse_cli.hpp"
#include "catalyst/subcommands/init/action.hpp"
#include "catalyst/subcommands/init/parse_cli.hpp"
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <string>

int main(int argc, char **argv) {
    CLI::App app{"Catalyst is a Modern Declarative C++ Build System."};
    const auto [add_subc, add_res] = catalyst::add::parse(app);
    const auto [init_subc, init_res] = catalyst::init::parse(app);
    CLI11_PARSE(app, argc, argv);
    if (*add_subc) {
        auto res = catalyst::add::action(*add_res);
        if (!res) {
            return 1;
        }
    }
    if (*init_subc) {
        auto res = catalyst::init::action(*init_res);
        if (!res) {
            return 1;
        }
    }
    return 0;
}
