#pragma once
#include "catalyst/subcommands/run/parse_cli.hpp"
#include <expected>
#include <string>

namespace catalyst::run {
std::expected<void, std::string> action(const parse_t &);
}
