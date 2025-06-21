#include "parse_rules.h"
#include "subcommand_actions.h"
#include "utils.h"

#include <CLI/CLI.hpp>

int main(int argc, char **argv) {
    CLI::App app{"Catalyst is a modern build system for Modern C++."};
    CLI::App *add_sub = app.add_subcommand("add", "Add a dependency to the Catalyst project manifest.");
    CLI::App *init_sub = app.add_subcommand("init", "Initialize the current directory as a Catalyst project.");
    CLI::App *build_sub = app.add_subcommand("build", "Generates the build file for the current Catalyst project.");
    add_args_t a_args;
    init_args_t i_args;
    build_args_t b_args;
    parse_rules_add(add_sub, a_args);
    parse_rules_init(init_sub, i_args);
    parse_rules_build(build_sub, b_args);
    app.get_formatter()->column_width(80);

    try {
        app.parse(argc, argv);
    } catch (const CLI ::ParseError &e) {
        if (strcmp(argv[1], "--help") && strcmp(argv[1], "-h"))
            log_print(log_level::ERROR, "Failed to parse cli arguments");
        return app.exit(e);
    };

    if (*add_sub)
        subcommand_add_action(a_args);
    else if (*init_sub)
        subcommand_init_action(i_args);
    else if (*build_sub) {
        // these are a bit too complex to validate using straight CLI11 so I use this
        // function defined in parse_rules.h
        if (!validate_build_args(b_args))
            return 1;
        subcommand_build_action(b_args);
    }
    return 0;
}
