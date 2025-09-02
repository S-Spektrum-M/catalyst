#pragma once
#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

// ANSI color codes
inline const char *RED = "\033[31m";
inline const char *RESET = "\033[0m";

namespace catalyst {
enum class LogLevel { INFO, WARN, ERROR };

// Helper function to convert LogLevel to string
inline const char *to_string(LogLevel level) {
    switch (level) {
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::WARN:
        return "WARN";
    case LogLevel::ERROR:
        return "ERROR";
    }
    return "UNKNOWN";
}

class log_t {
  public:
    static log_t &instance() {
        static log_t logger_instance;
        return logger_instance;
    }

    log_t(const log_t &) = delete;
    log_t &operator=(const log_t &) = delete;

    template <typename... Args> void log(LogLevel level, std::format_string<Args...> fmt, Args &&...args) {
        if (!log_file_.is_open()) {
            return;
        }

        auto now = std::chrono::system_clock::now();
        std::string message = std::format(fmt, std::forward<Args>(args)...);

        log_file_ << std::format("[{:%Y-%m-%d %H:%M:%S}] [{}] {}\n", now, to_string(level), message);

        if (level == LogLevel::ERROR) {
            std::cerr << std::format("[{:%Y-%m-%d %H:%M:%S}] ", now) << RED         // Start color
                      << std::format("[{}] {}", to_string(level), message) << RESET // Reset to default color
                      << "\n";
        }
        log_file_.flush(); // Ensure logs are written immediately
    }

    bool is_open() const { return log_file_.is_open(); }

    void close() {
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

  private:
    log_t() : log_file_{".catalyst.log", std::ios_base::app} {}
    ~log_t() {
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

    std::ofstream log_file_;
};

// Global logger instance
inline log_t &logger = log_t::instance();

} // namespace catalyst
