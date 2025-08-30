#include "catalyst/subcommands/generate.hpp"
#include <catalyst/hooks.hpp>
#include <catalyst/subcommands/clean.hpp>
#include <yaml-cpp/node/node.h>

namespace catalyst::clean {
std::expected<void, std::string> action(const parse_t &parse_args) {
    auto profiles = parse_args.profiles;
    if (std::find(profiles.begin(), profiles.end(), "common") == profiles.end()) {
        profiles.insert(profiles.begin(), "common");
    }

    YAML::Node profile_comp;
    if (auto res = generate::profile_composition(profiles); !res) {
        return std::unexpected(res.error());
    } else {
        profile_comp = res.value();
    }

    if (auto res = hooks::pre_clean(profile_comp); !res) {
        return res;
    }

    std::string build_dir = profile_comp["manifest"]["dirs"]["build"].as<std::string>();
    if (std::system(std::format("ninja -C {} -t clean", build_dir).c_str()) != 0) {
        return std::unexpected("error in cleaning.");
    }

    if (auto res = hooks::post_clean(profile_comp); !res) {
        return res;
    }

    return {};
}
} // namespace catalyst::clean
