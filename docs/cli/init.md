# catalyst init

```
Initialize a new catalyst profile.

Usage: catalyst init [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -n,--name TEXT              the name of the project
  --path TEXT                 the default path for the project
  -t,--type TEXT              the project type {binary, staticlib, sharedlib, interface}
  -v,--version TEXT           the project's version
  -d,--description TEXT       a description for the project
  --provides TEXT             Artifact provided by this project.
  --cc TEXT                   the c compiler to use
  --cxx TEXT                  the cxx compiler to use
  --ccflags TEXT              c compiler flags
  --cxxflags TEXT             cxx compiler flags
  --include-dirs TEXT ...     include directories
  --source-dirs TEXT ...      source directories
  --build-dir TEXT            build directory
  -p,--profile TEXT           the profile to initialize
```

## Examples

**Create a basic binary project:**
```bash
catalyst init --name my-tool
```

**Create a library with specific compilers:**
```bash
catalyst init --name my-lib --type staticlib --cxx g++-13
```

**Create a debug profile:**
```bash
catalyst init --profile debug --cxxflags "-g -O0"
```
