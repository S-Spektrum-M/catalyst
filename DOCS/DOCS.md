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
| `type`        | One of `BINARY`, `STATICLIB`, `SHAREDLIB`, or `HEADER` | `BINARY` |
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

The `dependencies` section lists external packages your project depends on.

Each entry is an object with the following keys:

|Field|Description|
|---------|---------------------------------------------------------------------|
|`name`|Name of the dependency|
|`version`|SemVer string or Git commit hash (hash preferred for reproducibility)|
|`source`|Git URL, `catalyst-hub`, or `vcpkg`|
|`using`|List of feature flags to enable in the dependency|

> [!NOTE]
> Catalyst resolves dependencies directly from Git where possible, enabling minimal and reproducible builds.

> [!NOTE]
> The SemVer version must correspond to a valid git tag.
> The git hash must be a valid commit.

### `features` Schema

Features are named preprocessor flags that can control build-time behavior.

- Each feature is a boolean and corresponds to a `#define` macro.
- Features can be validated at compile time using `static_assert`.
- They can be toggled by profiles or CLI overrides.

> [!NOTE]
> Catalyst automatically defines the macro `CATALYST_BUILD_SYS` to signal builds run under Catalyst. If your project
> defines this macro manually, collisions may occur.

### `hooks` Schema

> [!NOTE]
> Hooks will be added to Catalyst in `0.0.2`.

Hooks are optional scripts triggered at various stages of the build lifecycle. They enable incorporating support for
imperative configs to adapt to the realities of cross-platform development. Support is planned for bash, powershell,
and python scripts.

|Field|Description|
|-----|-----------|
|`prebuild`|Executed before a target is compiled|
|`postbuild`|Executed after a successful build|
|`fail`|Executed if the build fails|
|`publish`|Custom logic for uploading artifacts or pushing metadata|


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
