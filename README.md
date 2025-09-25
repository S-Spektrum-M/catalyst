<table border="0">
  <tr>
    <td><h1>Catalyst: A Declarative Build System for C/C++</h1></td>
    <td align="right"><img src="logo.svg" alt="catalyst-logo" width="150"></td>
  </tr>
</table>

## Introduction

Catalyst is a declarative meta build system for C and C++ that reimagines project configuration and dependency management around clarity, determinism, and reproducibility.

Designed to be simple yet powerful, Catalyst emphasizes per-target isolation, clean YAML-based configuration, and strict reproducibility guarantees by default. Unlike traditional tools like CMake or Autotools, Catalyst avoids ad hoc scripting and imperative logic, favoring profiles, feature flags, and an extensible schema that aligns with modern systems programming practices.

## Docs

Documentation can be found [here](docs/index.md) or by running `catalyst help`.

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

## License

This project is licensed to you under the Apache 2.0 [License](./LICENSE.md).
