#pragma once
#include <chrono>
#include <format>
#include <fstream>
#include <string>

namespace catalyst {
enum class LogLevel { INFO, WARN, ERROR };

inline struct log_t {
    std::ofstream log_file{".catalyst.log"};
    template <typename... Args> void log(LogLevel level, std::format_string<Args...> fmt, Args &&...args) {
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        std::println(
            log_file, "{} {} {}", std::chrono::system_clock::now(),
            [](LogLevel l) -> const char * {
                switch (l) {
                case LogLevel::INFO:
                    return "INFO";
                case LogLevel::WARN:
                    return "WARN";
                case LogLevel::ERROR:
                    return "ERROR";
                }
            }(level),
            message);
        log_file << std::chrono::system_clock::now();
    }

    ~log_t() { log_file.close(); }
} logger;
} // namespace catalyst
