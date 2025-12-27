# Profiles

Profiles are a core concept in Catalyst, allowing you to define different build configurations
(e.g., Debug, Release, Testing) and compose them together.

## Defining Profiles

- **`catalyst.yaml`**: Defines the base `common` profile.
- **`catalyst_<name>.yaml`**: Defines a profile named `<name>`.

### Example

**`catalyst.yaml` (common)**
```yaml
manifest:
  name: my-app
  version: 1.0.0
```

**`catalyst_debug.yaml` (debug)**
```yaml
manifest:
  tooling:
    CXXFLAGS: "-g -O0 -DDEBUG"
```

**`catalyst_release.yaml` (release)**
```yaml
manifest:
  tooling:
    CXXFLAGS: "-O3 -DNDEBUG"
```

## Composition

Catalyst allows you to compose profiles by listing them. Settings are merged from left to right, with later profiles overriding earlier ones.

```bash
catalyst build --profiles debug release
```

In this example:
1.  Start with `common`.
2.  Merge `debug` settings.
3.  Merge `release` settings (overriding `debug` where conflicts occur).

This allows for powerful combinations like `linux + debug + asan`.

## Nulling Fields

To unset a value inherited from a previous profile, set it to `null`.

```yaml
# catalyst_custom.yaml
meta: null  # Removes 'meta' section entirely
```

## Reserved Profiles

- **`common`**: The base profile (`catalyst.yaml`).
- **`test`**: Automatically used by `catalyst test` (`catalyst_test.yaml`).
