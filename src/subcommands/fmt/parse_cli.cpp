#include "catalyst/subcommands/fmt.hpp"

namespace catalyst::fmt {
std::pair<CLI::App *, std::unique_ptr<Parse>> parse(CLI::App &app) {
    CLI::App *fmt = app.add_subcommand("fmt", "Format project source files.");
    auto ret = std::make_unique<Parse>();
    return {fmt, std::move(ret)};
}
} // namespace catalyst::fmt
