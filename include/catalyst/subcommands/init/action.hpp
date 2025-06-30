#pragma once
#include "catalyst/subcommands/init/parse_cli.hpp"
#include <expected>
#include <string>

namespace catalyst::init {
std::expected<void, std::string> action(const parse_t &);
}
