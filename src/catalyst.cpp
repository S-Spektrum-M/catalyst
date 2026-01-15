#include "catalyst/GLOBALS.hpp"
#include "catalyst/dispatch.hpp"
#include "catalyst/log-utils/log.hpp"

#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <format>
#include <print>
#include <string>

namespace {
std::string concatArgv(int argc, char **argv) {
    std::string res;
    for (int ii = 0; ii < argc; ++ii)
        res += std::string{argv[ii]} + " ";
    return res;
}
} // namespace

int main(int argc, char **argv) {
    std::string args_str = concatArgv(argc, argv);
    catalyst::logger.log(catalyst::LogLevel::DEBUG, "{}", args_str);

    catalyst::CliContext ctx;
    auto [exit_code, should_return] = catalyst::parseCli(argc, argv, ctx);
    if (should_return)
        return exit_code;

    if (ctx.show_version) {
        std::println("{}", catalyst::CATALYST_VERSION);
        return 0;
    }

    return dispatch(ctx, args_str);
}
