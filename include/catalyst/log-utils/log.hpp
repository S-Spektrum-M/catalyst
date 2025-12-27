#pragma once
#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
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
inline const char *to_string(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
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
        std::lock_guard<std::mutex> lock(log_file_mutex_);
        if (!log_file_.is_open()) {
            return;
        }

        auto now = std::chrono::system_clock::now();
        std::string message = std::format(fmt, std::forward<Args>(args)...);

        log_file_ << generate_json_log_event(now, level, message) << "\n";

        if (verbose_logging || level != LogLevel::DEBUG) {
            const char *color = RESET;
            switch (level) {
                case LogLevel::DEBUG:
                    color = PURPLE;
                    break;
                case LogLevel::INFO:
                    color = BLUE;
                    break;
                case LogLevel::WARN:
                    color = ORANGE;
                    break;
                case LogLevel::ERROR:
                    color = RED;
                    break;
            }

            if (level == LogLevel::ERROR) {
                std::lock_guard<std::mutex> lock{stdio_mutex};
                std::cerr << std::format("[{:%Y-%m-%d %H:%M:%S}] ", now) << color
                          << std::format("[{}] {}", to_string(level), message) << RESET << "\n";
            } else {
                std::lock_guard<std::mutex> lock{stdio_mutex};
                std::cout << std::format("[{:%Y-%m-%d %H:%M:%S}] ", now) << color
                          << std::format("[{}] {}", to_string(level), message) << RESET << "\n";
            }
        }
    }

    bool is_open() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(log_file_mutex_));
        return log_file_.is_open();
    }

    void flush() {
        std::lock_guard<std::mutex> lock(log_file_mutex_);
        log_file_.flush();
    }

    void close() {
        std::lock_guard<std::mutex> lock(log_file_mutex_);
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

private:
    log_t() : log_file_{".catalyst.log", std::ios_base::app} {
        auto now = std::chrono::system_clock::now();
        nlohmann::json j;
        j["event"] = "begin_session";
        j["timestamp"] = std::format("{:%Y-%m-%d %H:%M:%S}", now);
        log_file_ << j.dump() << "\n";
    }
    ~log_t() {
        auto now = std::chrono::system_clock::now();
        // Destructor assumes single thread or end of life
        nlohmann::json j;
        j["event"] = "end_session";
        j["timestamp"] = std::format("{:%Y-%m-%d %H:%M:%S}", now);
        log_file_ << j.dump() << "\n";
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

    std::string generate_json_log_event(const std::chrono::system_clock::time_point &now,
                                        LogLevel level,
                                        const std::string &message) {
        nlohmann::json j;
        j["timestamp"] = std::format("{:%Y-%m-%d %H:%M:%S}", now);
        j["level"] = to_string(level);
        j["message"] = message;
        return j.dump();
    }

    std::ofstream log_file_;
    std::mutex log_file_mutex_;
    std::mutex stdio_mutex; // it's safe to reuse this since writes to stderr and stdout in this class happen seperately

public:
    bool verbose_logging = false;
};

// Global logger instance
inline log_t &logger = log_t::instance();

} // namespace catalyst
