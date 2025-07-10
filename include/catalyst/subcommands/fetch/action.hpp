#pragma once
#include "catalyst/subcommands/fetch/parse_cli.hpp"
#include <expected>
#include <string>

namespace catalyst::fetch {
std::expected<void, std::string> action(const parse_t &);
}
