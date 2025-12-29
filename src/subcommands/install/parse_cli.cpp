#include "catalyst/subcommands/install.hpp"

#include <CLI/App.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>

namespace catalyst::install {
std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app) {
    CLI::App *install = app.add_subcommand("install", "Install the build artifacts");
    auto ret = std::make_unique<parse_t>();
    install->add_option("-p,--profiles", ret->profiles, "the profiles to compose in the build artifact")
        ->default_val(std::vector<std::string>{"common"});
    install->add_option("-s,--source", ret->source_path, "the source of the path to build")->default_val(std::filesystem::current_path());
    install->add_option("-t,--target", ret->target_path, "the path to install to")->required();
    return {install, std::move(ret)};
}
} // namespace catalyst::install
