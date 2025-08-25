# Preprocessor Variables

Catalyst leverages the C++ preprocessor to control build configurations and inject valuable information into your source code. This is achieved by defining and undefining macros using the `-D` and `-U` compiler flags, respectively. This mechanism allows for powerful compile-time customization, enabling features like conditional compilation, feature flags, and build-time metadata.

## Catalyst-Defined Variables

Catalyst automatically defines several preprocessor variables to provide build-time information and control.

### Build System Indicator

Catalyst defines a special variable to indicate that the build is running within the Catalyst ecosystem. This is useful
for writing code that needs to be compatible with multiple build systems.

| Macro | Description |
| :--- | :--- |
| `CATALYST_BUILD_SYS` | Always defined by Catalyst. You can use `#ifdef CATALYST_BUILD_SYS` to write code that is specific to the Catalyst build environment. |
| `CATALYST_PROJ_NAME` | Defined by Catalyst as `manifest.name` of the final profile composition. |
| `CATALYST_PROJ_VER` | Defined by Catalyst as `manifest.version` of the final profile composition. |


### Feature Flags

Features are a core concept in Catalyst, allowing you to enable or disable specific functionalities in your code at
compile time. They are defined in the `features` section of your `catalyst.yaml` profile.

When a feature is enabled, Catalyst defines a corresponding preprocessor macro.

**Naming Convention:**

```cpp
FF_<PROJECT_NAME>__<FEATURE_NAME>
```

-   `<PROJECT_NAME>`: The name of your project, as defined in `manifest.name`.
-   `<FEATURE_NAME>`: The name of the feature, as defined in the `features` section.

**Example:**

If your `catalyst.yaml` contains:

```yaml
manifest:
  name: "my_project"

features:
  - logging: true
  - fastmath: false
```

If we, say, build with logging turned on but not fastmath, we get the following:

```sh
-DFF_my_project__logging=1 -DFF_my_project__fastmath=0
```

You can then use this in your code:

```cpp
#if FF_my_project__logging
  // Logging-specific code
#endif
```

## User-Defined Variables

While Catalyst's built-in variables cover many use cases, you can also define your own custom preprocessor variables through compiler flags.

You can add custom preprocessor variables to the `CCFLAGS` and `CXXFLAGS` fields in `manifest.tooling` section of `catalyst.yaml`.

**Example:**

```yaml
manifest:
  tooling:
    CXXFLAGS: "-DNDEBUG -DMY_CUSTOM_VARIABLE=42"
```

This will define `NDEBUG` and set `MY_CUSTOM_VARIABLE` to `42`, which you can then use in your code:

```cpp
#ifndef NDEBUG
  std::cout << "Debugging is enabled." << std::endl;
#endif

#if MY_CUSTOM_VARIABLE > 40
  // Do something special
#endif
```

By combining Catalyst's automatic variables with your own custom definitions, you can create highly flexible and configurable C++ projects.
