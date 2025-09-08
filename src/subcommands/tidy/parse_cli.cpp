#include "catalyst/subcommands/tidy.hpp"

namespace catalyst::tidy {

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app) {
    CLI::App *tidy = app.add_subcommand("tidy", "run linting");
    auto ret = std::make_unique<parse_t>();
    tidy->add_option("-p,--profiles", ret->profiles);
    return {tidy, std::move(ret)};
}
} // namespace catalyst::tidy
