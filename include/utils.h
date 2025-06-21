#include <format>
#include <fstream>
#include <ios>
#include <iostream>
#include <print>
#include <string>
#include <utility>
#include <yaml-cpp/node/node.h>

enum class log_level { INFO, WARN, DEBUG, ERROR };

constexpr const char *GREEN = "\033[32m";
constexpr const char *YELLOW = "\033[33m";
constexpr const char *MAGENTA = "\033[35m";
constexpr const char *RED = "\033[31m";
constexpr const char *RESET = "\033[0m";

template <typename... Args> void log_print(log_level ll, std::format_string<Args...> fmt, Args... args) {
  std::ofstream tty{"/dev/tty"};
  tty << std::unitbuf;
  switch (ll) {
  case log_level::INFO:
    tty << GREEN << "INFO: " << RESET;
    break;
  case log_level::WARN:
    tty << YELLOW << "WARN: " << RESET;
    break;
  case log_level::DEBUG:
    tty << MAGENTA << "DEBUG: " << RESET;
    break;
  case log_level::ERROR:
    tty << RED << "ERROR: " << RESET;
    break;
  }
  std::cout << std::format(fmt, std::forward<Args>(args)...) << std::endl;
}

template<typename... Keys>
YAML::Node node_deref(YAML::Node& manifest, Keys... keys) {
    YAML::Node current = manifest;
    ( (current = current[keys]), ... );
    return current;
}
