#include "catalyst/log-utils/log.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

namespace catalyst {

const char *to_string(LogLevel level) {
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

log_t::log_t() : log_file_{ ".catalyst.log", std::ios_base::app } {
    auto now = std::chrono::system_clock::now();
    nlohmann::json j;
    j["event"] = "begin_session";
    j["timestamp"] = std::format("{:%Y-%m-%d %H:%M:%S}", now);
    log_file_ << j.dump() << "\n";
}

log_t::~log_t() {
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

bool log_t::is_open() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(log_file_mutex_));
    return log_file_.is_open();
}

void log_t::flush() {
    std::lock_guard<std::mutex> lock(log_file_mutex_);
    log_file_.flush();
}

void log_t::close() {
    std::lock_guard<std::mutex> lock(log_file_mutex_);
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

void log_t::log_impl(LogLevel level, const std::string &message) {
    std::lock_guard<std::mutex> lock(log_file_mutex_);
    if (!log_file_.is_open()) {
        return;
    }

    auto now = std::chrono::system_clock::now();
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

std::string log_t::generate_json_log_event(const std::chrono::system_clock::time_point &now,
                                           LogLevel level,
                                           const std::string &message) {
    nlohmann::json j;
    j["timestamp"] = std::format("{:%Y-%m-%d %H:%M:%S}", now);
    j["level"] = to_string(level);
    j["message"] = message;
    return j.dump();
}

} // namespace catalyst
