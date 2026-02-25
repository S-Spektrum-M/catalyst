#pragma once
#include <CLI/App.hpp>
#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace catalyst::init {
struct Parse {
    enum class Type : std::uint8_t { BINARY, STATICLIB, SHAREDLIB, INTERFACE };
    enum class IdeType : std::uint8_t { vsc, clion };

    std::string name{std::filesystem::current_path().filename().string()};
    std::filesystem::path path{std::filesystem::current_path()};
    Type type{Parse::Type::BINARY};
    std::string version{"0.0.1"};
    std::string description{"Your description goes here."};
    std::string provides;
    struct {
        std::string cc{"clang"};
        std::string cxx{"clang++"};
        std::string ccflags;
        std::string cxxflags;
    } tooling;
    struct {
        std::vector<std::string> include{{"include"}};
        std::vector<std::string> source{{"src"}};
        std::string build{"build"};
    } dirs;
    std::string profile{"common"}; // only allow initializing one profile at a time.
    std::vector<Parse::IdeType> ides;
};

std::pair<CLI::App *, std::unique_ptr<Parse>> parse(CLI::App &app);
std::expected<void, std::string> action(const Parse &);
template <Parse::IdeType Ide_T> std::expected<void, std::string> emitIDEConfig(const Parse& parse_args);
} // namespace catalyst::init
