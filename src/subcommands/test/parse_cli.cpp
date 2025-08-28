#include "catalyst/subcommands/test.hpp"

namespace catalyst::test {

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app) {
    CLI::App *test = app.add_subcommand("test", "test the project");
    auto ret = std::make_unique<parse_t>();
    test->add_option("-P,--params", ret->params);
    return {test, std::move(ret)};
}
} // namespace catalyst::test
