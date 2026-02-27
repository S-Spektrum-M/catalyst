# catalyst ide_sync

```
Sync IDE configuration files for an existing project.
Usage: catalyst ide_sync [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -p,--profiles TEXT [common]  ...
                              Profiles to use from the configuration file
  --ides ENUM:value in {clion->,vscode->} OR {} ...
                              IDEs to generate project files for
  -f,--force-ide [0]          force emitting IDE config even if one already exists
```

## Details

The `ide_sync` command regenerates IDE project files (such as VS Code or CLion configurations) based on the current Catalyst configuration. This is useful when you've modified your project's structure, dependencies, or build settings and need to update your IDE integration.

## Examples

**Sync IDE configurations:**
```bash
catalyst ide_sync
```

**Sync for specific IDE:**
```bash
catalyst ide_sync --ides vscode
```

**Force regenerate IDE config:**
```bash
catalyst ide_sync --force-ide
```

**Sync with specific profiles:**
```bash
catalyst ide_sync --profiles common debug
```

**Sync for multiple IDEs:**
```bash
catalyst ide_sync --ides vscode clion
```
