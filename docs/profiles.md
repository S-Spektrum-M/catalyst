# Understanding Profiles in Catalyst

## What are Profiles?

In Catalyst, a **profile** is a YAML file that declares how to build a specific target. Each target has a
corresponding profile, creating a one-to-one relationship. This design ensures that build configurations are explicit,
self-contained, and easy to manage.

For detailed documentation on the profile schema and all available fields, please refer to the
[Catalyst Profile Configuration Schema](CONFIGURATION.md).


## Reserved Profiles

The primary profile for a project is `catalyst.yaml`, which defines the `common` profile. You can define additional
profiles by creating files named `catalyst_{PROFILE_NAME}.yaml`. This convention also applies to the following special
profiles:


|Profile Name|File Name|Purpoose|
|------------|---------|--------|
|`test`|`catalyst_test.yaml`| Facilitate code testing. |


## Profile Composition

A powerful feature of Catalyst is **profile composition**, which allows you to create new build configurations by
inheriting and overriding existing profiles. This promotes reusability and reduces duplication.

When you compose profiles, they are merged from left to right. For example, if you compose `debug` and `release` as
`debug release`, the final configuration will be a combination of both, with any conflicting
fields from `release` overriding those in `debug`.

### Example of Composition

Let's consider a `debug` and a `release` profile.

**`catalyst_debug.yaml`:**
```yaml
# Defines a debug build
manifest:
  name: "catalyst-debug"
  tooling:
    CXXFLAGS: "-g -O0 -DDEBUG"
```

**`catalyst_release.yaml`:**
```yaml
# Defines a standard release build.
manifest:
  name: "catalyst"
  version: "0.1.0"
  tooling:
    CXXFLAGS: "-O3 -DNDEBUG"
```

If you build with the composition `debug release`, the resulting profile would be:
```yaml
# Combined profile: 'debug' overridden by 'release'
manifest:
  name: "catalyst" # Overridden by release
  version: "0.1.0" # Inherited from release
  tooling:
    CXXFLAGS: "-O3 -DNDEBUG" # Overridden by release
```

### Nulling Fields

If you need to explicitly unset a field during composition, you can set its value to `null`.

**`catalyst_debug.yaml`:**
```yaml
meta:
  min_ver: "0.0.2"
```

**`catalyst_release.yaml`:**
```yaml
# In the release profile, we want to remove the min_ver constraint.
meta: null
```

When composed as `debug release`, the `meta` field will be completely undefined in the final configuration.
