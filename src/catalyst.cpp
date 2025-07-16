#include "catalyst/subcommands/add/action.hpp"
#include "catalyst/subcommands/add/parse_cli.hpp"
#include "catalyst/subcommands/build/parse_cli.hpp"
#include "catalyst/subcommands/build/action.hpp"
#include "catalyst/subcommands/fetch/action.hpp"
#include "catalyst/subcommands/fetch/parse_cli.hpp"
#include "catalyst/subcommands/generate/action.hpp"
#include "catalyst/subcommands/generate/parse_cli.hpp"
#include "catalyst/subcommands/init/action.hpp"
#include "catalyst/subcommands/init/parse_cli.hpp"
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <string>

int main(int argc, char **argv) {
    CLI::App app{"Catalyst is a Modern Declarative C++ Build System."};
    const auto [add_subc, add_res] = catalyst::add::parse(app);
    const auto [init_subc, init_res] = catalyst::init::parse(app);
    const auto [generate_subc, generate_res] = catalyst::generate::parse(app);
    const auto [fetch_subc, fetch_res] = catalyst::fetch::parse(app);
    const auto [build_subc, build_res] = catalyst::build::parse(app);
    std::string catalyst_version{"0.0.1"};
    bool show_version{false};
    app.add_flag("-v,--version", show_version, "current version");
    app.add_subcommand("help", "Display help information for a subcommand.")->callback([&]() {
        std::cout << app.help() << std::endl;
    });
    CLI11_PARSE(app, argc, argv);
    if (show_version) {
        std::cout << catalyst_version << std::endl;
    }
    if (*add_subc)
        if (auto res = catalyst::add::action(*add_res); !res)
            return 1;
    if (*init_subc)
        if (auto res = catalyst::init::action(*init_res); !res)
            return 1;
    if (*generate_subc)
        if (auto res = catalyst::generate::action(*generate_res); !res)
            return 1;
    if (*fetch_subc)
        if (auto res = catalyst::fetch::action(*fetch_res); !res)
            return 1;
    if (*build_subc) {
        if (auto res = catalyst::build::action(*build_res); !res)
            return 1;
    }
    return 0;
}
