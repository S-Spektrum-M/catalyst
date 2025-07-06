#pragma once
#include "catalyst/subcommands/generate/parse_cli.hpp"
#include <expected>
#include <string>

namespace catalyst::generate {
std::expected<void, std::string> action(const parse_t &);
}
