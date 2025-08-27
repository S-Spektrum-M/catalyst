#include "catalyst/subcommands/add.hpp"

namespace catalyst::add {

std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app) {
    CLI::App *add = app.add_subcommand("add", "add a dependency");
    auto ret = std::make_unique<parse_t>();
    add->add_option("name", ret->name);
    add->add_option("-v,--version", ret->version);
    add->add_option("-s,--src", ret->source);
    add->add_option("-p,--profiles", ret->profiles);
    add->add_option("-f,--features", ret->enabled_features);
    return {add, std::move(ret)};
}
} // namespace catalyst::add
