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
git clone https://github.com/CatalystCPP/catalyst-build-system.git
cd catalyst
```

2.  **Build the Bootstrap version using CMake:**

```bash
git checkout tags/1.0.0
cmake -B build-bootstrap -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build-bootstrap --config Release
```

3.  **Build Catalyst:**

```bash
git checkout dev-1.1.0
mkdir build
./build-bootstrap/catalyst build
```

## Verifying Installation

Run the following command to verify that Catalyst is installed correctly:

```bash
build/catalyst -v # should be 1.1.0
```

## Next Steps

- [**Get Started**](getting_started.md).
