#include "catalyst/subcommands/run.hpp"

namespace catalyst::run {

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app) {
    CLI::App *run = app.add_subcommand("run", "Run a built executable.");
    auto ret = std::make_unique<parse_t>();
    run->add_option("-p,--profile", ret->profile)->default_val("common");
    run->add_option("-P,--params", ret->params);
    return {run, std::move(ret)};
}
} // namespace catalyst::run
