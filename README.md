# Catalyst: A Declarative Build System for C/C++

![catalyst-logo](logo.svg)

## Introduction

Catalyst is a declarative meta build system for C and C++ that reimagines project configuration and dependency management around clarity, determinism, and reproducibility.

Designed to be simple yet powerful, Catalyst emphasizes per-target isolation, clean YAML-based configuration, and strict reproducibility guarantees by default. Unlike traditional tools like CMake or Autotools, Catalyst avoids ad hoc scripting and imperative logic, favoring profiles, feature flags, and an extensible schema that aligns with modern systems programming practices.

## Features

- **Declarative Targets** — Each target is specified independently using a clean, readable `catalyst.yaml`.
- **Single-Target Output** — Each directory maps to one target; complex projects are composed via directory layout.
- **Reproducibility First** — Deterministic builds with minimal external assumptions.
- **Feature Flags** — Compile-time options toggled via a `features` field.
- **Profile Composition** — Reuse and extend common settings declaratively through named profiles.
- **Cross-Compilation Ready** — Designed with toolchain switching and target platform isolation in mind.
- **Git-Based Dependency Resolution** — Projects are fetched directly from source with minimal overhead.
- **First-Class Testing Support** — Integration with lightweight test frameworks and custom test runners.

## Configuration

Read [this page](DOCS/CONFIGURATION.md)

## Docs

Documentation can be found [here](./DOCS.md) or by running `catalyst help`.

## Acknowledgements

Catalyst is very heavily inspired by the following projects:

- [Cargo](https://github.com/rust-lang/cargo)
- [Ninja](https://ninja-build.org/)
- [vcpkg](https://vcpkg.io/en/)

Catalyst also depends on the following libraries:

- [yaml-cpp](https://github.com/jbeder/yaml-cpp)
- [CLI11](https://github.com/CLIUtils/CLI11)

## Contributors

- Maintainer: [Siddharth Mohanty](https://www.linkedin.com/in/siddharth---mohanty)
