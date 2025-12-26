#include "catalyst/GLOBALS.hpp"
#include "catalyst/subcommands/add.hpp"
#include "catalyst/subcommands/build.hpp"
#include "catalyst/subcommands/clean.hpp"
#include "catalyst/subcommands/configure.hpp" // deprecated
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
    const auto [fetch_subc, fetch_res] = catalyst::fetch::parse(app);
    const auto [fmt_subc, fmt_res] = catalyst::fmt::parse(app);
    const auto [generate_subc, generate_res] = catalyst::generate::parse(app);
    const auto [init_subc, init_res] = catalyst::init::parse(app);
    const auto [run_subc, run_res] = catalyst::run::parse(app);
    const auto [test_subc, test_res] = catalyst::test::parse(app);
    const auto [tidy_subc, tidy_res] = catalyst::tidy::parse(app);
    const auto [add_git_subc, add_git_res] = catalyst::add::git::parse(*add_subc);
    const auto [add_system_subc, add_system_res] = catalyst::add::system::parse(*add_subc);
    const auto [add_local_subc, add_local_res] = catalyst::add::local::parse(*add_subc);
    const auto [add_vcpkg_subc, add_vcpkg_res] = catalyst::add::vcpkg::parse(*add_subc);

    bool show_version{false};

    app.add_flag("-v,--version", show_version, "current version");
    app.add_flag("-V,--verbose", catalyst::logger.verbose_logging, "verbose stdout logging output");

    bool helped = false;
    app.add_subcommand("help", "Display help information for a subcommand.")->callback([&]() {
        std::cout << app.help() << std::endl;
        helped = true;
    });
    if (helped)
        return 0; // avoid catalyst <subcommand> help from triggering the subcommand

    try {
        (app).parse(argc, argv);
    } catch (const CLI ::ParseError &e) {
        auto help_command = [](int argc, const char *const *argv) -> bool {
            for (int ii = 0; ii < argc; ++ii) {
                if (strcmp(argv[ii], "--help") == 0 || strcmp(argv[ii], "-h") == 0)
                    return true;
            }
            return false;
        };
        if (!help_command(argc, argv)) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "Failed to parse provided arguments: {}", concat_argv());
            return (app).exit(e);
        } else {
            return (app).exit(e);
        }
    };

    if (show_version) {
        std::print(std::cout, "{}", catalyst::CATALYST_VERSION);
        return 0;
    }

    if (*add_subc) {
        if (*add_git_subc) {
            catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "add git");
            if (std::expected<void, std::string> res = catalyst::add::git::action(*add_git_res); !res) {
                catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
                return 1;
            }
        } else if (*add_system_subc) {
            catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "add system");
            if (std::expected<void, std::string> res = catalyst::add::system::action(*add_system_res); !res) {
                catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
                return 1;
            }
        } else if (*add_local_subc) {
            catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "add local");
            if (std::expected<void, std::string> res = catalyst::add::local::action(*add_local_res); !res) {
                catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
                return 1;
            }
        } else if (*add_vcpkg_subc) {
            catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "add vcpkg");
            if (std::expected<void, std::string> res = catalyst::add::vcpkg::action(*add_vcpkg_res); !res) {
                catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
                return 1;
            }
        }
    }

    if (*build_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "build");
        if (std::expected<void, std::string> res = catalyst::build::action(*build_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }

    if (*clean_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "clean");
        if (std::expected<void, std::string> res = catalyst::clean::action(*clean_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }

    if (*fetch_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "fetch");
        if (std::expected<void, std::string> res = catalyst::fetch::action(*fetch_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }

    if (*fmt_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "fmt");
        if (std::expected<void, std::string> res = catalyst::fmt::action(*fmt_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }

    if (*generate_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "generate");
        if (std::expected<void, std::string> res = catalyst::generate::action(*generate_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }

    if (*init_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "init");
        if (std::expected<void, std::string> res = catalyst::init::action(*init_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }

    if (*run_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "run");
        if (std::expected<void, std::string> res = catalyst::run::action(*run_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }

    if (*test_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "test");
        if (std::expected<void, std::string> res = catalyst::test::action(*test_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }

    if (*tidy_subc) {
        catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", "tidy");
        if (std::expected<void, std::string> res = catalyst::tidy::action(*tidy_res); !res) {
            catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
            return 1;
        }
    }

    return 0;
}
