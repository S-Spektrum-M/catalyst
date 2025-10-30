# Catalyst

> ⚠️ **Early Stage**: Catalyst is under active development. Expect breaking changes and rough edges.

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE.md)

---

## What is Catalyst?

Catalyst is a declarative build system that brings Cargo-like simplicity to C and C++. No more wrestling with CMake scripts or Makefiles—just clean YAML configuration and reproducible builds.

**Why Catalyst?**

- **Declarative, not imperative** – YAML config files instead of scripting languages
- **Profile-based builds** – Compose configurations like `debug + asan` or `release + lto`
- **Built-in dependency management** – Native support for vcpkg, git repos, local packages, and system libraries
- **Reproducible by default** – Per-target isolation with deterministic builds
- **Modern C++ focused** – Designed for C++20/23/26 workflows

Think Cargo for Rust, but for C++.

---

## Quick Start

### Installation

```bash
# Clone the repository
git clone https://github.com/S-Spektrum-M/catalyst.git
cd catalyst

# Build Catalyst using CMake
cmake -B build -G Ninja
cmake --build build
cmake --install build
```

### Create Your First Project

```bash
# Initialize a new C++ project
mkdir my-project
cd my-project
catalyst init my-project

# YOU: write to any file in src/

# Build it
catalyst build

# Run it
catalyst run
```

### Example Project Structure

```
my-project/
├── catalyst.yaml          # Main configuration
├── src/
│   └── main.cpp
├── include/
│   └── my-project/
└── build/                 # Generated build files
```

**`catalyst.yaml`:**
```yaml
manifest:
  name: my-project
  type: BINARY
  version: 0.1.0
  tooling:
    CXX: clang++
    CXXFLAGS: -std=c++20 -Wall -Wextra
  dirs:
    include: [include]
    source: [src]
    build: build
```

---

## Key Features

### Profile Composition

Create reusable build configurations and compose them:

```yaml
# catalyst_debug.yaml
manifest:
  tooling:
    CXXFLAGS: -g -O0 -DDEBUG

# catalyst_asan.yaml
manifest:
  tooling:
    CXXFLAGS: -fsanitize=address
```

Build with composed profiles:
```bash
catalyst build --profiles debug,asan
```

### Dependency Management

Support for multiple dependency sources:

```yaml
dependencies:
  # From vcpkg
  - name: boost
    source: vcpkg
    version: 1.82.0

  # From git
  - name: nlohmann-json
    source: https://github.com/nlohmann/json.git
    version: v3.11.2

  # Local packages
  - name: my-lib
    source: local
    path: ../my-lib

  # System packages
  - name: openssl
    source: system
```

### Feature Flags

Compile-time feature toggles:

```yaml
features:
  - logging: true
  - networking: false
```

Access in code:
```cpp
#if FF_my_project__logging
  std::cout << "Logging enabled" << std::endl;
#endif
```

### Build Hooks

Run custom commands at different build stages:

```yaml
hooks:
  pre_build: "echo 'Starting build...'"
  post_build:
    - command: "./scripts/generate_docs.sh"
    - script: "./scripts/notify_slack.sh"
```

### Smart Source Management

Exclude files with `.catalystignore`:

```yaml
# src/.catalystignore
debug:
  - test_*.cpp
  - *_benchmark.cpp

release:
  - *_debug.cpp
```

---

## Documentation

- **[Configuration Guide](docs/configuration.md)** – Complete YAML schema reference
- **[Profiles](docs/profiles.md)** – How to use and compose profiles
- **[Hooks](docs/hooks.md)** – Lifecycle callbacks and scripting
- **[Project Structure](docs/project_structure.md)** – Code organization for contributors

Or run:
```bash
catalyst help
catalyst help <subcommand>
```

---

## Commands

| Command | Description |
|---------|-------------|
| `catalyst init` | Create a new project |
| `catalyst build` | Build the project |
| `catalyst run` | Run the binary |
| `catalyst test` | Run tests |
| `catalyst clean` | Clean build artifacts |
| `catalyst fmt` | Format source code |
| `catalyst tidy` | Run linter |
| `catalyst add` | Add a dependency |
| `catalyst fetch` | Fetch dependencies |

See the [roadmap](ROADMAP.md) for planned features.

---

## Why Not CMake?

CMake is powerful but complex. Catalyst trades some of that power for simplicity and developer experience:

| Feature | CMake | Catalyst |
|---------|-------|----------|
| Configuration | Imperative scripting | Declarative YAML |
| Learning curve | Steep | Gentle |
| Dependency management | Manual or external | Built-in |
| Profile composition | Complex | Native |
| Reproducibility | Requires discipline | Default behavior |

**When to use Catalyst:**
- New projects where you control the build system
- Projects that value simplicity and maintainability
- Teams tired of CMake complexity

**When to use CMake:**
- Existing large codebases
- Need for complex custom build logic
- Projects requiring broad platform/toolchain support

---

## Current Limitations

- **Early stage** – APIs will change, bugs exist
- **Linux/macOS focus** – Windows support is limited
- **Single target per project** – Workspaces are planned but not implemented
- **No package registry** – Currently uses vcpkg, git, and local dependencies

See [ROADMAP.md](ROADMAP.md) for what's coming.

---

## Contributing

Catalyst is in active development and contributions are welcome!

**Before contributing:**
1. Open an issue to discuss major changes
2. Expect API churn in these early stages
3. Check the [project structure guide](docs/project_structure.md)

**Development setup:**
```bash
git clone https://github.com/S-Spektrum-M/catalyst.git
cd catalyst
catalyst build --profiles debug
catalyst test
```

---

## Inspiration & Thanks

Catalyst draws inspiration from:
- **[Cargo](https://github.com/rust-lang/cargo)** – Profile system and declarative config
- **[Ninja](https://ninja-build.org/)** – Fast incremental builds
- **[vcpkg](https://vcpkg.io/en/)** – Modern C++ package management

Built with:
- [yaml-cpp](https://github.com/jbeder/yaml-cpp) – YAML parsing
- [CLI11](https://github.com/CLIUtils/CLI11) – Command-line interface

---

## License

Apache 2.0 – See [LICENSE.md](LICENSE.md)

---

## Contact

**Maintainer:** [Siddharth Mohanty](https://www.linkedin.com/in/siddharth---mohanty)

Questions or feedback? Open an issue or reach out at [neosiddharth@gmail.com](mailto:neosiddharth@gmail.com).
