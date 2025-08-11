 Road Map
# DONE (0.0.0 - 0.0.1)

- [x] add                  Add dependencies to a catalyst.yaml manifest file
- [x] init                 Create a new catalyst package in an existing directory
- [x] generate             Generate the build file
- [x] fetch                Fetch and compile dependencies from the network
- [x] build                Invoke Ninja on the build path.


## 0.0.2
- [ ] tidy                 Checks a package using an external `clang-tidy` call to catch common mistakes.
- [ ] clean                Remove artifacts that catalyst has generated in the past
- [ ] config               Inspect configuration values
- [ ] doc                  Build a package's documentation
- [ ] fix                  Automatically fix lint warnings reported by clang-tidy

## Out of Scope of 0.0.2
- [ ] fmt                  Formats all source files.
- [ ] generate-lockfile    Generate the lockfile for a package
- [ ] metadata             Output the resolved dependencies of a package, the concrete used versions including overrides, in machine-readable format miri
- [ ] owner                Manage the owners of a crate on the registry
- [ ] package              Assemble the local package into a distributable tarball
- [ ] pkgid                Print a fully qualified package specification
- [ ] publish              Upload a package to the registry
- [ ] remove               Remove dependencies from a Cargo.toml manifest file
- [ ] report               Generate and display various kinds of reports
- [ ] run                  Run a binary or example of the local package
- [ ] rustc                Compile a package, and pass extra options to the compiler
- [ ] rustdoc              Build a package's documentation, using specified custom flags.
- [ ] search               Search packages in the registry. Default registry is crates.io
- [ ] test                 Execute all unit and integration tests and build examples of a local package
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
- [ ] install              Install a Rust binary
- [ ] uninstall            Remove a Rust binary
- [ ] bench                Execute all benchmarks of a local package
