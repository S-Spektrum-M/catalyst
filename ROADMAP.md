#  Road Map

| Subcommand | Status | Description |
|------------|--------|-------------|
| `add` | Done | Add dependencies to a catalyst.yaml manifest file |
| `init` | Done | Create a new catalyst package in an existing directory |
| `generate` | Done | Generate the build file |
| `fetch` | Done | Fetch and compile dependencies from the network |
| `build` | Done | Invoke Ninja on the build path. |
| `run` | Done | Run a binary profile's target. |
| `test` | Done | Execute all unit and integration tests of a profile. |
| `config` | Done | Inspect and modify configuration values |
| `clean` | Done | Remove artifacts that catalyst has generated in the past |
| `fmt` | Done | Formats all source files. |
| `tidy` | Done | Checks a package using an external `clang-tidy` call to catch common mistakes. |
| `fix` | Planned | Automatically fix lint warning reported by `clang-tidy`. |
| `pack` | Planned | Assemble the local package for distribution. |
| `bench` | Planned | Execute all benchmarks of a local package. |
| `doc` | Planned | Build a package's documentation. |
| `generate-lockfile` | Planned | Generate the lockfile for a package. |
| `metadata` | Planned | Output the resolved dependecies of a package, the concrete used versions including overrides, in machine-readable format miri. |
| `owner` | Planned | Manage the owners of a package on the registry. |
| `publish` | Planned | Uplaot a package to the registry. |
| `remove` | Planned | Remove dependencies from a Cargo.toml manifest file. |
| `tree` | Planned | Display a tree visualization of a dependency graph. |
| `report` | Out of Scope | Generate and display various kinds of reports |
| `update` | Out of Scope | Update dependencies as recorded in the local lock file |
| `vendor` | Out of Scope | Vendor all dependencies for a project locally |
| `version` | Out of Scope | Show version information |
| `yank` | Out of Scope | Remove a pushed package from the index |
| `help` | Out of Scope | Displays help for a catalyst subcommand |
| `info` | Out of Scope | Display information about a package |
| `locate-project` | Out of Scope | Print a JSON representation of a Cargo.toml file's location |
| `login` | Out of Scope | Log in to a registry. |
| `logout` | Out of Scope | Remove an API token from the registry locally |
| `install` | Out of Scope | Install a binary |
| `uninstall` | Out of Scope | Remove a binary |
| `pkgid` | Out of Scope | Print a fully qualified package specification |
| `search` | Out of Scope | Search packages in the registry. Default registry is TBD. |


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
