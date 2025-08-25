#pragma once
#include "catalyst/subcommands/test/parse_cli.hpp"
#include <expected>
#include <string>

namespace catalyst::test {
    std::expected<void, std::string> action(const parse_t &);
}
