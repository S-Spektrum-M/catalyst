#include "catalyst/subcommands/add.hpp"

namespace catalyst::add {

std::pair<CLI::App *, std::unique_ptr<Parse>> parse(CLI::App &app) {
    CLI::App *add = app.add_subcommand("add", "Add a dependency.");
    auto ret = std::make_unique<Parse>();
    return {add, std::move(ret)};
}
} // namespace catalyst::add
