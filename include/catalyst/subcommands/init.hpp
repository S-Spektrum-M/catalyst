#pragma once
#include <CLI/App.hpp>
#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace catalyst::init {
struct parse_t {
    enum class Type { BINARY, STATICLIB, SHAREDLIB, INTERFACE };

    std::string name{std::filesystem::current_path().filename().string()};
    std::filesystem::path path{std::filesystem::current_path()};
    Type type{parse_t::Type::BINARY};
    std::string version{"0.0.1"};
    std::string description{"Your description goes here."};
    std::string provides{""};
    struct {
        std::string CC{"clang"};
        std::string CXX{"clang++"};
        std::string CCFLAGS{""};
        std::string CXXFLAGS{""};
    } tooling;
    struct {
        std::vector<std::string> include{{"include"}};
        std::vector<std::string> source{{"src"}};
        std::string build{"build"};
    } dirs;
    std::string profile{"common"}; // only allow initializing one profile at a time.
};

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app);
std::expected<void, std::string> action(const parse_t &);
} // namespace catalyst::init
