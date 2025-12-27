# catalyst generate

```
generate a build script
Usage: catalyst generate [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -p,--profiles TEXT ...      
  -f,--features TEXT ...      
```

## Details

This command translates the declarative YAML configuration into a concrete Ninja build plan. It resolves source files, include paths, compile flags, and dependency links. It is typically invoked automatically by `catalyst build`.