#include "catalyst/subcommands/tidy.hpp"
#include <expected>
#include <string>

namespace catalyst::tidy {
std::expected<void, std::string> action(const parse_t &) {
    return {};
}
}
