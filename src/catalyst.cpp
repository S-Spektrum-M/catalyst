#include "catalyst/GLOBALS.hpp"
#include "catalyst/subcommands/add/action.hpp"
#include "catalyst/subcommands/add/parse_cli.hpp"
#include "catalyst/subcommands/build/action.hpp"
#include "catalyst/subcommands/build/parse_cli.hpp"
#include "catalyst/subcommands/clean/action.hpp"
#include "catalyst/subcommands/clean/parse_cli.hpp"
#include "catalyst/subcommands/configure/action.hpp"
#include "catalyst/subcommands/configure/parse_cli.hpp"
#include "catalyst/subcommands/fetch/action.hpp"
#include "catalyst/subcommands/fetch/parse_cli.hpp"
#include "catalyst/subcommands/generate/action.hpp"
#include "catalyst/subcommands/generate/parse_cli.hpp"
#include "catalyst/subcommands/init/action.hpp"
#include "catalyst/subcommands/init/parse_cli.hpp"
#include "catalyst/subcommands/run/action.hpp"
#include "catalyst/subcommands/run/parse_cli.hpp"
#include "catalyst/subcommands/test/action.hpp"
#include "catalyst/subcommands/test/parse_cli.hpp"
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <chrono>
#include <string>

int main(int argc, char **argv) {
    CLI::App app{"Catalyst is a Modern Declarative C++ Build System."};

    const auto [add_subc, add_res] = catalyst::add::parse(app);
    const auto [build_subc, build_res] = catalyst::build::parse(app);
    const auto [clean_subc, clean_res] = catalyst::clean::parse(app);
    const auto [configure_subc, configure_res] = catalyst::configure::parse(app);
    const auto [fetch_subc, fetch_res] = catalyst::fetch::parse(app);
    const auto [generate_subc, generate_res] = catalyst::generate::parse(app);
    const auto [init_subc, init_res] = catalyst::init::parse(app);
    const auto [run_subc, run_res] = catalyst::run::parse(app);
    const auto [test_subc, test_res] = catalyst::test::parse(app);

    bool show_version{false};

    app.add_flag("-v,--version", show_version, "current version");

    app.add_subcommand("help", "Display help information for a subcommand.")->callback([&]() {
        std::cout << app.help() << std::endl;
    });

    CLI11_PARSE(app, argc, argv);

    if (show_version) {
        std::cout << catalyst::CATALYST_VERSION << std::endl;
    }

    if (*add_subc) {
        if (auto res = catalyst::add::action(*add_res); !res) {
            std::cerr << res.error() << std::endl;
            return 1;
        }
    }

    if (*build_subc) {
        if (auto res = catalyst::build::action(*build_res); !res) {
            std::cerr << res.error() << std::endl;
            return 1;
        }
    }

    if (*clean_subc) {
        if (auto res = catalyst::clean::action(*clean_res); !res) {
            std::cerr << res.error() << std::endl;
            return 1;
        }
    }
    if (*configure_subc) {
        if (auto res = catalyst::configure::action(*configure_res); !res) {
            std::cerr << res.error() << std::endl;
            return 1;
        }
    }

    if (*fetch_subc) {
        if (auto res = catalyst::fetch::action(*fetch_res); !res) {
            std::cerr << res.error() << std::endl;
            return 1;
        }
    }

    if (*generate_subc) {
        if (auto res = catalyst::generate::action(*generate_res); !res) {
            std::cerr << res.error() << std::endl;
            return 1;
        }
    }

    if (*init_subc) {
        if (auto res = catalyst::init::action(*init_res); !res) {
            std::cerr << res.error() << std::endl;
            return 1;
        }
    }

    if (*run_subc) {
        if (auto res = catalyst::run::action(*run_res); !res) {
            std::cerr << res.error() << std::endl;
            return 1;
        }
    }

    if (*test_subc) {
        if (auto res = catalyst::test::action(*test_res); !res) {
            std::cerr << res.error() << std::endl;
            return 1;
        }
    }

    return 0;
}
