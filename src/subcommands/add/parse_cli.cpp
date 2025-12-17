#include "catalyst/subcommands/add.hpp"

namespace catalyst::add {

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app) {
    CLI::App *add = app.add_subcommand("add", "add a dependency");
    auto ret = std::make_unique<parse_t>();
    return {add, std::move(ret)};
}
} // namespace catalyst::add
