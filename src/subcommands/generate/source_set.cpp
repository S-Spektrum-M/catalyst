#include "catalyst/log-utils/log.hpp"
#include <expected>
#include <filesystem>
#include <ranges>
#include <regex>
#include <string>
#include <sys/wait.h>
#include <unordered_set>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace catalyst::generate {

namespace fs = std::filesystem;

std::expected<std::unordered_set<fs::path>, std::string> build_source_set(std::vector<std::string> source_dirs,
                                                                          const std::vector<std::string> &profiles) {
    catalyst::logger.log(LogLevel::INFO, "Building source set.");
    namespace sv = std::views;
    auto str_to_path = [](const std::string &str) { return fs::path{str}; };
    auto paths = source_dirs | sv::transform(str_to_path) | std::ranges::to<std::vector>();
    std::unordered_set<fs::path> source_set;
    std::unordered_set<std::string> ignored_files;

    for (const auto &dir : paths) {
        catalyst::logger.log(LogLevel::INFO, "Processing source directory: {}", dir.string());
        if (!fs::exists(dir) || !fs::is_directory(dir)) {
            catalyst::logger.log(LogLevel::ERROR, "Source directory not found or is not a directory: {}", dir.string());
            return std::unexpected(std::format("Source directory not found or is not a directory: {}", dir.string()));
        }

        fs::path ignore_file = dir / ".catalystignore";
        if (fs::exists(ignore_file)) {
            catalyst::logger.log(LogLevel::INFO, "Found .catalystignore file in: {}", dir.string());
            YAML::Node ignore_config = YAML::LoadFile(ignore_file.string());
            for (const auto &profile : profiles) {
                if (ignore_config[profile]) {
                    for (const auto &ignore_pattern : ignore_config[profile]) {
                        std::string pattern = ignore_pattern.as<std::string>();
                        catalyst::logger.log(LogLevel::INFO, "Ignoring pattern: {}", pattern);
                        ignored_files.insert(pattern);
                    }
                }
            }
        }

        for (const auto &entry : fs::recursive_directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                bool ignored = false;
                for (const auto &pattern : ignored_files) {
                    std::regex ignore_regex(pattern);
                    if (std::regex_match(entry.path().filename().string(), ignore_regex)) {
                        catalyst::logger.log(LogLevel::INFO, "Ignoring file: {}", entry.path().string());
                        ignored = true;
                        break;
                    }
                }
                if (!ignored) {
                    const auto &path = entry.path();
                    const std::string extension = path.extension().string();
                    if (extension == ".cpp" || extension == ".cxx" || extension == ".cc" || extension == ".c" || extension == ".cu" || extension == ".cupp") {
                        catalyst::logger.log(LogLevel::INFO, "Adding file to source set: {}", path.string());
                        source_set.insert(path);
                    }
                }
            }
        }
    }
    catalyst::logger.log(LogLevel::INFO, "Source set built successfully.");
    return source_set;
}
} // namespace catalyst::generate
