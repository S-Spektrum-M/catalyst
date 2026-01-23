#include "catalyst/dispatch.hpp"

#include "catalyst/log-utils/log.hpp"

namespace {
std::string concatArgv(int argc, char **argv) {
    std::string res;
    for (int ii = 0; ii < argc; ++ii)
        res += std::string{argv[ii]} + " ";
    return res;
}
} // namespace

namespace catalyst {
std::pair<int, bool> parseCli(int argc, char **argv, catalyst::CliContext &ctx) {
    auto [add_subc, add_res] = catalyst::add::parse(ctx.app);
    ctx.add_subc = add_subc;
    ctx.add_res = std::move(add_res);

    auto [build_subc, build_res] = catalyst::build::parse(ctx.app);
    ctx.build_subc = build_subc;
    ctx.build_res = std::move(build_res);

    auto [clean_subc, clean_res] = catalyst::clean::parse(ctx.app);
    ctx.clean_subc = clean_subc;
    ctx.clean_res = std::move(clean_res);

    auto [download_subc, download_res] = catalyst::download::parse(ctx.app);
    ctx.download_subc = download_subc;
    ctx.download_res = std::move(download_res);

    auto [fetch_subc, fetch_res] = catalyst::fetch::parse(ctx.app);
    ctx.fetch_subc = fetch_subc;
    ctx.fetch_res = std::move(fetch_res);

    auto [fmt_subc, fmt_res] = catalyst::fmt::parse(ctx.app);
    ctx.fmt_subc = fmt_subc;
    ctx.fmt_res = std::move(fmt_res);

    auto [generate_subc, generate_res] = catalyst::generate::parse(ctx.app);
    ctx.generate_subc = generate_subc;
    ctx.generate_res = std::move(generate_res);

    auto [init_subc, init_res] = catalyst::init::parse(ctx.app);
    ctx.init_subc = init_subc;
    ctx.init_res = std::move(init_res);

    auto [install_subc, install_res] = catalyst::install::parse(ctx.app);
    ctx.install_subc = install_subc;
    ctx.install_res = std::move(install_res);

    auto [run_subc, run_res] = catalyst::run::parse(ctx.app);
    ctx.run_subc = run_subc;
    ctx.run_res = std::move(run_res);

    auto [test_subc, test_res] = catalyst::test::parse(ctx.app);
    ctx.test_subc = test_subc;
    ctx.test_res = std::move(test_res);

    auto [tidy_subc, tidy_res] = catalyst::tidy::parse(ctx.app);
    ctx.tidy_subc = tidy_subc;
    ctx.tidy_res = std::move(tidy_res);

    auto [add_git_subc, add_git_res] = catalyst::add::git::parse(*ctx.add_subc);
    ctx.add_git_subc = add_git_subc;
    ctx.add_git_res = std::move(add_git_res);

    auto [add_system_subc, add_system_res] = catalyst::add::system::parse(*ctx.add_subc);
    ctx.add_system_subc = add_system_subc;
    ctx.add_system_res = std::move(add_system_res);

    auto [add_local_subc, add_local_res] = catalyst::add::local::parse(*ctx.add_subc);
    ctx.add_local_subc = add_local_subc;
    ctx.add_local_res = std::move(add_local_res);

    auto [add_vcpkg_subc, add_vcpkg_res] = catalyst::add::vcpkg::parse(*ctx.add_subc);
    ctx.add_vcpkg_subc = add_vcpkg_subc;
    ctx.add_vcpkg_res = std::move(add_vcpkg_res);

    ctx.app.add_flag("-v,--version", ctx.show_version, "current version");
    ctx.app.add_flag("-V,--verbose", catalyst::logger.verbose_logging, "verbose stdout logging output");

    ctx.app.add_subcommand("help", "Display help information for a subcommand.")->callback([&]() {
        std::cout << ctx.app.help() << '\n';
        ctx.helped = true;
    });

    try {
        ctx.app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        auto help_command = [](int argc, const char *const *argv) -> bool {
            for (int ii = 0; ii < argc; ++ii) {
                if (strcmp(argv[ii], "--help") == 0 || strcmp(argv[ii], "-h") == 0)
                    return true;
            }
            return false;
        };
        if (!help_command(argc, argv)) {
            catalyst::logger.log(
                catalyst::LogLevel::ERROR, "Failed to parse provided arguments: {}", concatArgv(argc, argv));
            return {ctx.app.exit(e), true};
        }
        return {ctx.app.exit(e), true};
    }

    if (ctx.helped)
        return {0, true};

    return {0, false};
}

template <typename ParseRes_T> int dispatchFN(const char *subc_name, const ParseRes_T &parse_res, auto fn) {
    catalyst::logger.log(catalyst::LogLevel::DEBUG, "Executing {} subcommand", subc_name);
    if (std::expected<void, std::string> res = fn(parse_res); !res) {
        catalyst::logger.log(catalyst::LogLevel::ERROR, "{}", res.error());
        return 1;
    }
    return 0;
}

int dispatch(const catalyst::CliContext &ctx, const std::string &args_str) {
    if (*ctx.add_subc) {
        if (*ctx.add_git_subc)
            return dispatchFN("add git", *ctx.add_git_res, catalyst::add::git::action);
        if (*ctx.add_system_subc)
            return dispatchFN("add system", *ctx.add_system_res, catalyst::add::system::action);
        if (*ctx.add_local_subc)
            return dispatchFN("add local", *ctx.add_local_res, catalyst::add::local::action);
        if (*ctx.add_vcpkg_subc)
            return dispatchFN("add vcpkg", *ctx.add_vcpkg_res, catalyst::add::vcpkg::action);
        return 1;
    }
    if (*ctx.build_subc)
        return dispatchFN("build", *ctx.build_res, catalyst::build::action);
    if (*ctx.clean_subc)
        return dispatchFN("clean", *ctx.clean_res, catalyst::clean::action);
    if (*ctx.download_subc)
        return dispatchFN("download", *ctx.download_res, catalyst::download::action);
    if (*ctx.fetch_subc)
        return dispatchFN("fetch", *ctx.fetch_res, catalyst::fetch::action);
    if (*ctx.fmt_subc)
        return dispatchFN("fmt", *ctx.fmt_res, catalyst::fmt::action);
    if (*ctx.generate_subc)
        return dispatchFN("generate", *ctx.generate_res, catalyst::generate::action);
    if (*ctx.init_subc)
        return dispatchFN("init", *ctx.init_res, catalyst::init::action);
    if (*ctx.install_subc)
        return dispatchFN("install", *ctx.install_res, catalyst::install::action);
    if (*ctx.run_subc)
        return dispatchFN("run", *ctx.run_res, catalyst::run::action);
    if (*ctx.test_subc)
        return dispatchFN("test", *ctx.test_res, catalyst::test::action);
    if (*ctx.tidy_subc)
        return dispatchFN("tidy", *ctx.tidy_res, catalyst::tidy::action);
    catalyst::logger.log(catalyst::LogLevel::ERROR, "run catalyst --help for info on available commands.", args_str);
    return 1;
}
} // namespace catalyst
