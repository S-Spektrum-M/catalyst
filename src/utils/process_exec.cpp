#include "catalyst/process_exec.h"

#include <expected>
#include <future>
#include <optional>
#include <reproc++/run.hpp>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

/// async wrapper around std::system()
/// TODO: migrate to safe exec functions like in reprco
namespace catalyst {
std::expected<std::future<int>, std::string>
process_exec(std::vector<std::string> &&args, std::optional<std::string> working_dir,
             std::optional<std::unordered_map<std::string, std::string>> env) {
    if (args.empty()) {
        return std::unexpected("Cannot execute empty command");
    }

    return std::async(std::launch::async, [args = std::move(args), working_dir, env]() -> int {
        reproc::options options;
        options.redirect.out.type = reproc::redirect::parent;
        options.redirect.err.type = reproc::redirect::parent;

        if (working_dir) {
            options.working_directory = working_dir->c_str();
        }

        std::vector<std::string> env_strings;
        std::vector<const char *> env_ptrs;
        if (env) {
            options.env.behavior = reproc::env::extend;
            for (const auto &[key, value] : *env) {
                env_strings.push_back(key + "=" + value);
            }
            for (const auto &s : env_strings) {
                env_ptrs.push_back(s.c_str());
            }
            env_ptrs.push_back(nullptr);
            options.env.extra = env_ptrs.data();
        }

        auto [status, ec] = reproc::run(args, options);

        if (ec)
            return -1;
        return status;
    });
}
} // namespace catalyst
