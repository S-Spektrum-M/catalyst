#pragma once
#include <CLI/App.hpp>
#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace catalyst::init {
struct parse_t {
    enum class type_t { BINARY, STATICLIB, SHAREDLIB, INTERFACE };

    std::string name;
    std::filesystem::path path;
    type_t type;
    std::string version;
    std::string description;
    std::string provides;
    struct {
        std::string CC;
        std::string CXX;
        std::string CCFLAGS;
        std::string CXXFLAGS;
    } tooling;
    struct {
        std::vector<std::string> include;
        std::vector<std::string> source;
        std::string build{"build"};
    } dirs;
    std::string profile{"common"}; // only allow initializing one profile at a time.
};

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app);
std::expected<void, std::string> action(const parse_t &);
} // namespace catalyst::init
