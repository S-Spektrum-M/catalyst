#include "catalyst/subcommands/generate/parse_cli.hpp"
#include <catalyst/subcommands/clean/action.hpp>
#include <yaml-cpp/node/node.h>

namespace catalyst::clean {
std::expected<void, std::string> action(const parse_t &parse_args) {
    const std::vector<std::string> &profiles = parse_args.profiles;
    YAML::Node profile_comp;
    if (auto res = generate::profile_composition(profiles); !res) {
        return std::unexpected(res.error());
    } else {
        profile_comp = res.value();
    }
    // 1. build profile comp
    std::string build_dir = profile_comp["manifest"]["dirs"]["build"].as<std::string>();
    if (std::system(std::format("ninja -C {} -t clean", build_dir).c_str())) {
        return std::unexpected("error in cleaning.");
    } else {
        return {};
    }
}
} // namespace catalyst::clean
