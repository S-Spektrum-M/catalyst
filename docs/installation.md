# Installation

Currently, Catalyst must be built from source.

## Prerequisites

- **CMake**: 3.20 or newer
- **C++ Compiler**: Clang 16+ or GCC 13+ (Must support C++23)
- **Ninja**: Recommended build backend
- **Git**

## Building from Source

1.  **Clone the repository:**

```bash
git clone https://github.com/S-Spektrum-M/catalyst.git
cd catalyst
```

2.  **Build with CMake:**

```bash
cmake -B build
cmake --build build
```

3.  **Install:**

```bash
sudo cmake --install build
```

Or add the `build/catalyst` binary to your PATH manually.

## Verifying Installation

Run the following command to verify that Catalyst is installed correctly:

```bash
catalyst --version
```

> NOTE: The following additional dependencies are required and must be installed sperately: ninja, vcpkg, clang-format, clang-tidy.
