# catalyst generate

```
generate a build script
Usage: catalyst generate [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
    -p,--profiles TEXT ...      
    -f,--features TEXT ...      
    -b,--backend TEXT           Backend to use for generation (ninja, gmake, cbe)
  ```
  
  ## Details
  
  This command translates the declarative YAML configuration into a concrete build plan (e.g., Ninja build file, Makefile, or CBE manifest). It resolves source files, include paths, compile flags, and dependency links. It is typically invoked automatically by `catalyst build`.
  