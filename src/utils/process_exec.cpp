#include "catalyst/process_exec.h"

#include <expected>
#include <future>
#include <reproc++/run.hpp>
#include <string>
#include <utility>
#include <vector>

/// async wrapper around std::system()
/// TODO: migrate to safe exec functions like in reprco
namespace catalyst {
std::expected<std::future<int>, std::string> R_process_exec(std::vector<std::string> &&args) {
    if (args.empty()) {
        return std::unexpected("Cannot execute empty command");
    }

    return std::async(std::launch::async, [args = std::move(args)]() -> int {
        reproc::options options;
        options.redirect.out.type = reproc::redirect::parent;
        options.redirect.err.type = reproc::redirect::parent;

        auto [status, ec] = reproc::run(args, options);

        if (ec)
            return -1;
        return status;
    });
}
} // namespace catalyst
