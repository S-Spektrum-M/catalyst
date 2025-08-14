#include "catalyst/subcommands/run/action.hpp"
#include <catalyst/subcommands/test/action.hpp>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

namespace catalyst::test {
    std::expected<void, std::string> action(const parse_t &p) {
        return run::action({"test", p.params});
    }
} // namespace catalyst::test
