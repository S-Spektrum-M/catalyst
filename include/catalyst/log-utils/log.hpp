#pragma once
#include <chrono>
#include <format>
#include <fstream>
#include <mutex>
#include <string>

// ANSI color codes
inline const char *RED = "\033[31m";
inline const char *ORANGE = "\033[33m";
inline const char *BLUE = "\033[34m";
inline const char *PURPLE = "\033[35m";
inline const char *RESET = "\033[0m";

namespace catalyst {
enum class LogLevel {
    DEBUG, // hidden info (unless asked for opt in with -V)
    INFO,  // user facing milestones
    WARN,  // warnings
    ERROR  // errors
};

// Helper function to convert LogLevel to string
const char *to_string(LogLevel level);

class log_t {
public:
    static log_t &instance() {
        static log_t logger_instance;
        return logger_instance;
    }

    log_t(const log_t &) = delete;
    log_t &operator=(const log_t &) = delete;

    template <typename... Args> void log(LogLevel level, std::format_string<Args...> fmt, Args &&...args) {
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        log_impl(level, message);
    }

    bool is_open() const;
    void flush();
    void close();

    bool verbose_logging = false;

private:
    log_t();
    ~log_t();

    void log_impl(LogLevel level, const std::string &message);
    static std::string generate_json_log_event(const std::chrono::system_clock::time_point &now,
                                        LogLevel level,
                                        const std::string &message);

    std::ofstream log_file_;
    mutable std::mutex log_file_mutex_;
    std::mutex stdio_mutex;
};

// Global logger instance
inline log_t &logger = log_t::instance();

} // namespace catalyst
