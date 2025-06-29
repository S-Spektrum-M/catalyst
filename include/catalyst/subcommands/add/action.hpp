#pragma once
#include "catalyst/subcommands/add/parse_cli.hpp"
#include <expected>
#include <string>

namespace catalyst::add {
std::expected<void, std::string> action(const parse_t &);
}
