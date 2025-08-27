#pragma once
#include "catalyst/subcommands/clean/parse_cli.hpp"
#include <string>
#include <expected>

namespace catalyst::clean {
std::expected<void, std::string> action(const parse_t &parse_args);
} // namespace catalyst::clean
