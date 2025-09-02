# Catalyst Profile Configuration Schema

> [!TIP]
> If reading docs isn't your style, please look at catalyst's own profiles and ignores
> which are extensively annotated or the catalyst example repo:
> [Catalyzing C++](https://www.github.com/S-Spektrum-M/catalyst-demo), which provides an intuitive follow-along.

Each Catalyst project has profiles defined in a yaml profile-configuration file.
The default (`common`) profile is defined in `catalyst.yaml`. Other profiles can
be defined in files of the form: `catalyst_{PROFILE NAME}.yaml`.


| Field        | Description                                  |
| ------------ | -------------------------------------------- |
| `meta`       | Catalyst Metadata                            |
| `manifest`     | Project metadata and build layout            |
| `dependencies` | External packages and package specifications |
| `features`     | Preprocessor-level feature flags, useful for library authors to provide togglable behavior. |
| `hooks`        | Optional lifecycle callbacks                 |

### `meta` Schema

The `meta` section defines Catalyst specific information.

| Field | Description | Default |
|-------|-------------|---------|
| `min_ver` | The minimum required catalyst version to build the project. | Equivalent to ``catalyst --version`` |

### `manifest` Schema

The `manifest` section defines core information about your project and how it should be built.

| Field         | Description                                                   | Default |
| ------------- | ------------------------------------------------------------- |---------|
| `name`        | The name of the project (must be unique within a workspace)   | The `cwd` when the project was initialized |
| `type`        | One of `BINARY`, `STATICLIB`, `SHAREDLIB`, or `INTERFACE` | `BINARY` |
| `version`     | Project version | `0.0.1` |
| `description` | Human-readable summary of the project                         | "Your Description Goes Here" |
| `provides`    | Name or path of the final output artifact (e.g. `'*.so', '*.dll', '*.exe'` etc.), useful for library authors to enable finding the build output. | None. |
| `tooling`     | Toolchain overrides                               | See below |
| `dirs`        | Build-relevant directories                        | See below |

> [!NOTE]
> The `type` field must be written exactly as is in the table above.
> The `version` field may eventually be deprecated in favor of Git commit hashes for better reproducibility.

#### `tooling` Subschema

Override default compiler and flag settings:

| Field      | Description                       | Default |
| ---------- | --------------------------------- |-|
| `CC`       | C compiler (default: `clang`)     | `clang` |
| `CXX`      | C++ compiler (default: `clang++`) |`clang++`|
| `CCFLAGS`  | Flags passed to the C compiler    | empty string |
| `CXXFLAGS` | Flags passed to the C++ compiler  | empty string |

#### `dirs` Subschema

Define important source and build paths within the project:

| Field | Description | Default |
| --------- | ------------------------------------------------------------ | - |
| `include` | List of include directories used for `-I` | `{include}`|
| `source` | List of source file directories | `{src}` |
| `build` | Directory where build outputs should go | `build` |

> [!NOTE]
> `dirs.source` recursively finds all source files within the listed directories.
> Also see the `.catalystignore` section for excluding files found by `dirs.source`.


### `dependencies` Schema

The `dependencies` section lists external packages your project depends on. Each entry in the list is a YAML object that defines a single dependency. The `source` field is mandatory and determines the type of the dependency and the schema for the other fields.

Catalyst supports the following dependency sources: `local`, `system`, `vcpkg`, and `git`.

#### `local` dependencies

For dependencies that exist on your local filesystem. Catalyst will build them from source.

| Field      | Description                                      | Required |
|------------|--------------------------------------------------|----------|
| `name`     | Name of the dependency.                          | Yes      |
| `source`   | Must be `local`.                                 | Yes      |
| `path`     | Absolute or relative path to the dependency's root directory. | Yes      |
| `profiles` | A list of profiles to use when building the dependency. | No       |
| `using`    | A list of features to enable in the dependency.  | No       |

Example:
```yaml
- name: my-local-lib
  source: local
  path: ../my-local-lib
  profiles:
    - release
```

#### `system` dependencies

For dependencies managed by your system's package manager (e.g., apt, brew, etc.). Catalyst will try to find them using `pkg-config`.

| Field     | Description                                                              | Required |
|-----------|--------------------------------------------------------------------------|----------|
| `name`    | Name of the dependency. This should match the name known to `pkg-config`. | Yes      |
| `source`  | Must be `system`.                                                        | Yes      |
| `lib`     | Path to the library directory. If not provided, Catalyst will guess.     | No       |
| `include` | Path to the include directory. If not provided, Catalyst will guess.     | No       |

Example:
```yaml
- name: openssl
  source: system
```

#### `vcpkg` dependencies

For dependencies managed by `vcpkg`.

| Field     | Description                                     | Required |
|-----------|-------------------------------------------------|----------|
| `name`    | Name of the `vcpkg` package.                    | Yes      |
| `source`  | Must be `vcpkg`.                                | Yes      |
| `version` | The version of the package to use.              | No       |
| `using`   | A list of features to enable in the dependency. | No       |

Example:
```yaml
- name: yaml-cpp
  source: vcpkg
  version: 0.7.0
```

#### `git` dependencies

For dependencies hosted in a `git` repository. Catalyst will clone and build them.

| Field     | Description                                      | Required |
|-----------|--------------------------------------------------|----------|
| `name`    | Name of the dependency.                          | Yes      |
| `source`  | The URL of the git repository.                   | Yes      |
| `version` | A git tag or commit hash to checkout.            | Yes      |
| `using`   | A list of features to enable in the dependency.  | No       |

Example:
```yaml
- name: fmt
  source: https://github.com/fmtlib/fmt.git
  version: 8.1.1
```

### `features` Schema

```
features:
    - <FEATURE_NAME>: <DEFAULT_VALUE>
```

The `features` is a YAML list of togglable features, to control build-time behavior.
Flags can through arguments to the `generate` and `build ` subcommands.

- Each feature is a boolean and corresponds to a `#define` macro.
- Features can be validated at compile time using `static_assert`.
- They can be toggled by profiles or CLI overrides.

> [!NOTE]
> Catalyst automatically defines the macro `CATALYST_BUILD_SYS` to signal builds run under Catalyst. If your project
> defines this macro manually, collisions may occur.

### `hooks` Schema

Hooks are optional scripts triggered at various stages of the build lifecycle. They enable incorporating support for
imperative steps to adapt to the realities of development. For more information, check out [the hooks documentation](docs/hooks.md).


## `.catalystignore` File Schema

Catalyst allows users to specify entire directories as `source` directories.
However, this can result in unintended source files
(e.g., test stubs, platform-specific files) being included in a build.
To prevent this, Catalyst supports a `.catalystignore` file, similar in
spirit to `.gitignore` or `.dockerignore`.

When cerating the source file set, Catalyst looks for a `.catalystignore` file
inside each specified `source` directory. Any file matching a pattern in that
file will be excluded from the build.

### Syntax

```yaml
profile_name:               # the name of the profile to ignore for
    - ignored_file.cpp      # Explicitly ignore a file
    - *_windows.cpp         # Use glob patterns to ignore multiple files
```

- `profile_name` refers to a build profile defined in your configuration.
- Patterns follow basic glob syntax (e.g., `*`, `?`, `**/file.cpp`).

### Example

```yaml
common:
 - legacy_*.cpp
 - backup/*
release:
  - test_*.cpp
  - *_debug.cpp
debug:
  - benchmark.cpp
```

### Behavior

- If a profile is active and a matching section exists, its ignore rules are applied
in addition to the global ones.
- Files ignored via `.catalystignore` are completely excluded from the
compilation step and dependency graph.
