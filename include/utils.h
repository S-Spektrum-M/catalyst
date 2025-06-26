#include "globals.h"
#include <format>
#include <fstream>
#include <ios>
#include <iostream>
#include <utility>
#include <yaml-cpp/node/node.h>

enum log_level { INFO, WARN, ERROR };

constexpr const char *YELLOW = "\033[33m";
constexpr const char *RED = "\033[31m";
constexpr const char *RESET = "\033[0m";

template <typename... Args> void log_print(log_level ll, std::format_string<Args...> fmt, Args... args) {
    std::ofstream tty{"/dev/tty"};
    tty << std::unitbuf;
    std::string message = std::format(fmt, std::forward<Args>(args)...);
    switch (ll) {
    case log_level::INFO:
        logfile << "INFO: " << message << std::endl;
        break;
    case log_level::WARN:
        logfile << "WARN: " << message << std::endl;
        tty << YELLOW << "WARN: " << RESET;
        std::cout << "WARN: " << message << std::endl;
        break;
    case log_level::ERROR:
        logfile << "ERROR: " << message << std::endl;
        tty << RED << "ERROR: " << RESET;
        std::cout << "ERROR: " << message << std::endl;
        break;
    }
}
