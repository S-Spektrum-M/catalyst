# Catalyst Code Strucuture

```
src/
    catalyst.cpp            -- this represents the entry point to the catalyst executable
    ... TBD                 -- this represents the space for one off future source files
    subcommands/            -- catalyst is fundamentally a subcommand driven program, so it makes sense for the code structure to reflect that
        add/                -- this is a representative sample of most subcommands
            parse_cli.cpp   -- every subcommand has a `parse_cli.cpp` to define how we want to parse command line.
            action.cpp      -- this represents what to do, given a set of command line options and flags.
include/catalyst/
    ... TBD                 -- this represents the space for one off future header files
    subcommands/            -- catalyst is fundamentally a subcommand driven program, so it makes sense for the code structure to reflect that
        add/                -- this is a representative sample of most subcommands
            parse_cli.hpp   -- this defines a struct, {subcommand_name}_parse_t, to caputre parsed values, and a function, {subcommand_name}_parse_t parse(int argc, char **argv)
            action.hpp      -- this just provides a single function, void {subcommand_name}_exec(const {subcommand_name}_parse_t &);
    yaml-utils/             -- this provides a number of utility functions for wrangling and emmiting YAML(driven by the yaml-cpp library)
    log-utils/              -- this provides functioanlity for writing to a a logfile(see: docs/logfile.md)
```
