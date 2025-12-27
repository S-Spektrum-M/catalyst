# catalyst build

```
build the project
Usage: catalyst build [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -r,--no-regen               regenerate the build file
  --force-rebuild             recompile dependencies
  --force-refetch             refetch dependencies
  -p,--profiles TEXT ...      
  -f,--features TEXT ...      
```

## Examples

**Standard build:**
```bash
catalyst build
```

**Debug build:**
```bash
catalyst build --profiles debug
```

**Force clean build:**
```bash
catalyst build --force-rebuild
```