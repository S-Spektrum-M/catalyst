#include "catalyst/log-utils/log.hpp"
#include "catalyst/subcommands/generate.hpp"
#include "catalyst/subcommands/tidy.hpp"
#include "yaml-cpp/node/node.h"
#include <atomic>
#include <cstdlib>
#include <deque>
#include <expected>
#include <filesystem>
#include <format>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

namespace catalyst::tidy {
std::expected<void, std::string> action(const parse_t &parse_args) {
    YAML::Node profile_comp;
    if (auto res = catalyst::generate::profile_composition(parse_args.profiles); !res)
        return std::unexpected(res.error());
    else
        profile_comp = res.value();

    if (!profile_comp["manifest"]["tooling"]["LINTER"] || !profile_comp["manifest"]["tooling"]["LINTER"].IsScalar())
        return std::unexpected("feild: manifest.tooling.LINTER is not defined");

    std::string LINTER = profile_comp["manifest"]["tooling"]["LINTER"].as<std::string>();
    // call the linter on the source_set (we can expect clang-tidy like arg syntax) and go on about our day

    catalyst::logger.log(LogLevel::INFO, "Building source set.");

    namespace fs = std::filesystem;

    fs::path current_dir = fs::current_path();
    std::vector relative_source_dirs = profile_comp["manifest"]["dirs"]["source"].as<std::vector<std::string>>();
    std::vector<std::string> absolute_source_dirs;
    for (const auto &dir : relative_source_dirs) {
        absolute_source_dirs.push_back((current_dir / dir).string());
    }

    std::unordered_set<std::filesystem::path> source_set;
    if (auto source_set_res = generate::build_source_set(absolute_source_dirs, parse_args.profiles); !source_set_res) {
        catalyst::logger.log(LogLevel::ERROR, "Failed to build source set: {}", source_set_res.error());
        return std::unexpected(source_set_res.error());
    } else {
        source_set = source_set_res.value();
    }

    unsigned int num_threads = std::thread::hardware_concurrency();
    catalyst::logger.log(LogLevel::INFO, "Running linter on {} files using {} threads.", source_set.size(),
                         num_threads);

    std::queue<fs::path, std::deque<fs::path>> work_queue(std::deque<fs::path>(source_set.begin(), source_set.end()));
    std::atomic<bool> has_errors = false;
    std::mutex queue_mutex;
    std::vector<std::thread> threads;

    for (unsigned i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            while (true) {
                fs::path file_to_process;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    if (work_queue.empty()) {
                        break;
                    }
                    file_to_process = work_queue.front();
                    work_queue.pop();
                }

                std::string command = LINTER + " " + file_to_process.string();
                if (int res = std::system(command.c_str()); res != 0) {
                    catalyst::logger.log(LogLevel::ERROR, "Linter failed for {}: exit code {}",
                                         file_to_process.string(), res);
                    has_errors = true;
                }
            }
        });
    }

    for (auto &t : threads) {
        t.join();
    }

    if (has_errors) {
        return std::unexpected("Linter finished with errors.");
    }

    catalyst::logger.log(LogLevel::INFO, "Tidy subcommand finished successfully.");
    return {};
}
} // namespace catalyst::tidy
