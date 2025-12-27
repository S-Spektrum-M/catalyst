#include "catalyst/subcommands/fmt.hpp"

namespace catalyst::fmt {
std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app) {
    CLI::App *fmt = app.add_subcommand("fmt", "Format project source files.");
    auto ret = std::make_unique<parse_t>();
    return {fmt, std::move(ret)};
}
} // namespace catalyst::fmt
