# catalyst fetch

```
fetch all dependencies for a profile
Usage: catalyst fetch [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  --workspace                 Link local workspace members if available
  --profiles TEXT ...         
```

## Details

This command is usually run automatically by `catalyst build`, but can be run manually to prepare the environment (e.g., in CI/CD pipelines).

- **Git**: Clones repositories.
- **Vcpkg**: Installs packages.
- **System**: Verifies presence via pkg-config.
- **Workspace**: When `--workspace` is used, Catalyst checks if a dependency exists as a member of the current workspace. If found, it creates a symbolic link to the local folder instead of downloading the dependency.

## Examples

```bash
catalyst fetch
catalyst fetch --profiles debug
```

**Workspace fetch:**
Link local packages where possible.
```bash
catalyst fetch --workspace
```