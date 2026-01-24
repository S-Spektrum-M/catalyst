#include "catalyst/subcommands/test.hpp"

namespace catalyst::test {

std::pair<CLI::App *, std::unique_ptr<Parse>> parse(CLI::App &app) {
    CLI::App *test = app.add_subcommand("test", "Run the test executable.");
    auto ret = std::make_unique<Parse>();
    test->add_option("-P,--params", ret->params, "Params to pass to the test executable.");
    return {test, std::move(ret)};
}
} // namespace catalyst::test
