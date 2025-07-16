#pragma once
#include "catalyst/subcommands/build/parse_cli.hpp"
#include <expected>
#include <string>

namespace catalyst::build {
std::expected<void, std::string> action(const parse_t &);
}
