#  Road Map

## WIP 0.0.4
- [x] tidy                 Checks a package using an external `clang-tidy` call to catch common mistakes.
- [ ] fix                  Automatically fix lint warnings reported by `clang-tidy`
- [ ] pack                 Assemble the local package for distribution

## Out of Scope
- [ ] bench                Execute all benchmarks of a local package
- [ ] doc                  Build a package's documentation
- [ ] generate-lockfile    Generate the lockfile for a package
- [ ] metadata             Output the resolved dependencies of a package, the concrete used versions including overrides, in machine-readable format miri
- [ ] owner                Manage the owners of a crate on the registry
- [ ] pkgid                Print a fully qualified package specification
- [ ] publish              Upload a package to the registry
- [ ] remove               Remove dependencies from a Cargo.toml manifest file
- [ ] report               Generate and display various kinds of reports
- [ ] search               Search packages in the registry. Default registry is crates.io
- [ ] tree                 Display a tree visualization of a dependency graph
- [ ] update               Update dependencies as recorded in the local lock file
- [ ] vendor               Vendor all dependencies for a project locally
- [ ] version              Show version information
- [ ] yank                 Remove a pushed crate from the index
- [ ] help                 Displays help for a catalyst subcommand
- [ ] info                 Display information about a package
- [ ] locate-project       Print a JSON representation of a Cargo.toml file's location
- [ ] login                Log in to a registry.
- [ ] logout               Remove an API token from the registry locally
- [ ] install              Install a binary
- [ ] uninstall            Remove a binary

# DONE (0.0.0 - 0.0.3)

- [x] add                  Add dependencies to a catalyst.yaml manifest file
- [x] init                 Create a new catalyst package in an existing directory
- [x] generate             Generate the build file
- [x] fetch                Fetch and compile dependencies from the network
- [x] build                Invoke Ninja on the build path.
- [x] run                  Run a binary profile's target.
- [x] test                 Execute all unit and integration tests of a profile.
- [x] config               Inspect and modify configuration values
- [x] clean                Remove artifacts that catalyst has generated in the past
- [x] fmt                  Formats all source files.

## YAML Platforming and Abstraction Layer

This section outlines the plan to build a more robust and maintainable platform for handling YAML configuration files,
moving away from direct `yaml-cpp` calls in the subcommands.

### (DONE): Phase 1: Core Configuration Library

-   [x] Create a central `Configuration` class: This class will be the main entry point for accessing all configuration
data. It will hold the composed `YAML::Node` and provide type-safe methods to access its values.
-   [x] Implement type-safe accessors: The `Configuration` class will have methods like
`getString(key)`, `getInt(key)`, `getBool(key)`, `getStringVector(key)`, etc. These methods will handle the YAML node access and type conversion, including error handling.
-   [x] Add support for nested keys: The accessors should support nested keys, for example `getString("manifest.dirs.source")`.

### Phase 2: Refactor Subcommands

-   [ ] Refactor `generate` subcommand: The `generate` subcommand will be the first to be refactored to use the new `Configuration` class. This will involve replacing all direct `YAML::Node` access with calls to the new type-safe accessors.
-   [ ] Refactor `add` subcommand: Refactor the `add` subcommand to use the `Configuration` class for modifying the `catalyst.yaml` file.
-   [ ] Refactor remaining subcommands: Refactor all other subcommands that use `yaml-cpp` to use the new `Configuration` class.

### Phase 3: Advanced Features

-   [ ] Configuration validation: Implement a validation system to check the loaded configuration against a schema. This will ensure that all required fields are present and have the correct types.
-   [ ] Configuration caching: Implement a caching mechanism in the `ProfileManager` to avoid reloading and parsing the same YAML files multiple times.
-   [ ] Environment variable substitution: Add support for environment variable substitution in the YAML files, for example `source_dir: ${SRC_ROOT}/src`.
