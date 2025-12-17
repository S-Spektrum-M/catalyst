#include "catalyst/GLOBALS.hpp"
#include "catalyst/subcommands/add.hpp"
#include "catalyst/subcommands/build.hpp"
#include "catalyst/subcommands/clean.hpp"
#include "catalyst/subcommands/configure.hpp"
#include "catalyst/subcommands/fetch.hpp"
#include "catalyst/subcommands/fmt.hpp"
#include "catalyst/subcommands/generate.hpp"
#include "catalyst/subcommands/init.hpp"
#include "catalyst/subcommands/run.hpp"
#include "catalyst/subcommands/test.hpp"
#include "catalyst/subcommands/tidy.hpp"

#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <catalyst/log-utils/log.hpp>
#include <format>
#include <iostream>
#include <print>
#include <string>

int main(int argc, char **argv) {
    auto concat_argv = [&argc, &argv]() -> std::string {
        int ac = argc;
        std::string res;
        for (int ii = 0; ii < ac; ++ii)
            res += std::string{argv[ii]} + " ";
        return res;
    };
    catalyst::logger.log(catalyst::LogLevel::DEBUG, "{}", concat_argv());

    CLI::App app{"Catalyst is a Modern Declarative C++ Build System."};

    const auto [add_subc, add_res] = catalyst::add::parse(app);
    const auto [build_subc, build_res] = catalyst::build::parse(app);
    const auto [clean_subc, clean_res] = catalyst::clean::parse(app);
    const auto [configure_subc, configure_res] = catalyst::configure::parse(app);
    const auto [fetch_subc, fetch_res] = catalyst::fetch::parse(app);
    const auto [fmt_subc, fmt_res] = catalyst::fmt::parse(app);
    const auto [generate_subc, generate_res] = catalyst::generate::parse(app);
    const auto [init_subc, init_res] = catalyst::init::parse(app);
    const auto [run_subc, run_res] = catalyst::run::parse(app);
    const auto [test_subc, test_res] = catalyst::test::parse(app);
    const auto [tidy_subc, tidy_res] = catalyst::tidy::parse(app);
    const auto [add_git_subc, add_git_res] = catalyst::add::git::parse(*add_subc);

    bool show_version{false};

    app.add_flag("-v,--version", show_version, "current version");
    app.add_flag("-V,--verbose", catalyst::logger.verbose_logging, "verbose stdout logging output");

    app.add_subcommand("help", "Display help information for a subcommand.")->callback([&]() {
        std::cout << app.help() << std::endl;
    });

    try {
        (app).parse(argc, argv);
    } catch (const CLI ::ParseError &e) {
        catalyst::logger.log(catalyst::LogLevel::ERROR, "Failed to pass provided arguments: {}", concat_argv());
        return (app).exit(e);
    };

    if (show_version) {
        std::print(std::cout, "{}", catalyst::CATALYST_VERSION);
        return 0;
    }

    if (*add_subc) {
        if (*add_git_subc) {
            catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "add git");
            if (auto res = catalyst::add::git::action(*add_git_res); !res) {
                catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
                return 1;
            }
        } else if (false) { // TODO: add future TYPE subcommands
        }
    }

    if (*build_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "build");
        if (auto res = catalyst::build::action(*build_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }

    if (*clean_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "clean");
        if (auto res = catalyst::clean::action(*clean_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }
    if (*configure_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "configure");
        if (auto res = catalyst::configure::action(*configure_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }

    if (*fetch_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "fetch");
        if (auto res = catalyst::fetch::action(*fetch_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }

    if (*fmt_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "fmt");
        if (auto res = catalyst::fmt::action(*fmt_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }

    if (*generate_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "generate");
        if (auto res = catalyst::generate::action(*generate_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }

    if (*init_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "init");
        if (auto res = catalyst::init::action(*init_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }

    if (*run_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "run");
        if (auto res = catalyst::run::action(*run_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }

    if (*test_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "test");
        if (auto res = catalyst::test::action(*test_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }

    if (*tidy_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "tidy");
        if (auto res = catalyst::tidy::action(*tidy_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }

    return 0;
}
