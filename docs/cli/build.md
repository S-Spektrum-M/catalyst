# catalyst build

```
build the project
Usage: catalyst build [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -r,--no-regen               regenerate the build file
  --force-rebuild             recompile dependencies
  --force-refetch             refetch dependencies
  --workspace,--all           Build all members in the workspace
  -P,--package TEXT           Build a specific package from the root
  -p,--profiles TEXT ...      
  -f,--features TEXT ...      
```

## Details

When running with `--workspace` or `--all`, Catalyst determines the correct build order based on the dependencies between workspace members. It ensures that dependencies are built before the packages that rely on them.

## Examples

**Standard build:**
```bash
catalyst build
```

**Workspace build:**
Build all packages in the current workspace, automatically ordering them by dependency.
```bash
catalyst build --workspace
```

**Build specific package:**
Build only the `app` package and its dependencies within the workspace.
```bash
catalyst build --package app
```

**Debug build:**
```bash
catalyst build --profiles debug
```

**Force clean build:**
```bash
catalyst build --force-rebuild
```

**Enable features:**
```bash
catalyst build --features logging
```

**Disable features:**
```bash
catalyst build --features no-logging
```