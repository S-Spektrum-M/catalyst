#pragma once
#include "catalyst/subcommands/configure/parse_cli.hpp"
#include <expected>
#include <string>

namespace catalyst::configure {
std::expected<void, std::string> action(const parse_t &);
}
