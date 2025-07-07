# Road Map
- [x] add                  Add dependencies to a Cargo.toml manifest file
- [x] init                 Create a new cargo package in an existing directory
- [ ] new                  Create a new cargo package at <path>
- [ ] build                Compile a local package and all of its dependencies
- [ ] fetch                Fetch dependencies of a package from the network



## Out of Scope of 0.0.1
- [ ] bench                Execute all benchmarks of a local package
- [ ] clippy               Checks a package to catch common mistakes and improve your Rust code.
    - rename from clippy
- [ ] check                Check a local package and all of its dependencies for errors
- [ ] clean                Remove artifacts that cargo has generated in the past
- [ ] config               Inspect configuration values
- [ ] doc                  Build a package's documentation
- [ ] fix                  Automatically fix lint warnings reported by rustc
- [ ] fmt                  Formats all bin and lib files of the current crate using rustfmt.
- [ ] generate-lockfile    Generate the lockfile for a package
- [ ] login                Log in to a registry.
- [ ] logout               Remove an API token from the registry locally
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
- [ ] uninstall            Remove a Rust binary
- [ ] update               Update dependencies as recorded in the local lock file
- [ ] vendor               Vendor all dependencies for a project locally
- [ ] version              Show version information
- [ ] yank                 Remove a pushed crate from the index
- [ ] help                 Displays help for a cargo subcommand
- [ ] info                 Display information about a package
- [ ] install              Install a Rust binary
- [ ] locate-project       Print a JSON representation of a Cargo.toml file's location
