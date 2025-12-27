# catalyst add

```
add a dependency
Usage: catalyst add [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                   Print this help message and exit

Subcommands:
  git                         add a remote git dependency
  system                      add a system dependency
  local                       add a local dependency
  vcpkg                       add a vcpkg dependency
```

### git
```
add a remote git dependency
Usage: catalyst add git [OPTIONS] [remote]

Positionals:
  remote TEXT

Options:
  -h,--help                   Print this help message and exit
  -n,--name TEXT
  -v,--version TEXT [latest]
  -f,--features TEXT ...
  -p,--profiles TEXT ...
```

### system
```
add a system dependency
Usage: catalyst add system [OPTIONS] name

Positionals:
  name TEXT REQUIRED

Options:
  -h,--help                   Print this help message and exit
  -l,--lib TEXT
  -i,--inc TEXT
  -p,--profiles TEXT ...
```

### local
```
add a local dependency
Usage: catalyst add local [OPTIONS] name path

Positionals:
  name TEXT REQUIRED
  path TEXT REQUIRED

Options:
  -h,--help                   Print this help message and exit
  -p,--profiles TEXT ...
  -f,--features TEXT ...
```

### vcpkg
```
add a vcpkg dependency
Usage: catalyst add vcpkg [OPTIONS] name

Positionals:
  name TEXT REQUIRED

Options:
  -h,--help                   Print this help message and exit
  -t,--triplet TEXT REQUIRED
  -v,--version TEXT [latest]
  -p,--profiles TEXT [[common]]  ...
  -f,--features TEXT ...
```

## Examples

```bash
# Add fmt from git
catalyst add git https://github.com/fmtlib/fmt.git -v 10.1.0

# Add fmt from vcpkg
catalyst add vcpkg fmt -t x64-linux

# Add a local library to the debug profile
catalyst add local my-lib ../libs/my-lib -p debug
```
