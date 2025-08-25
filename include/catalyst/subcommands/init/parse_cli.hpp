#pragma once
#include <CLI/App.hpp>
#include <filesystem>
#include <string>
#include <vector>

namespace catalyst::init {
struct parse_t {
    std::string name{std::filesystem::current_path().filename().string()};
    std::filesystem::path path{std::filesystem::current_path()};
    enum class type_t { BINARY, STATICLIB, SHAREDLIB, HEADER } type{type_t::BINARY};
    std::string version{"0.0.1"};
    std::string description{"Your Description Goes Here"};
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
} // namespace catalyst::init
