# Catalyst Log File Documentation

The logfile is organized into blocks that appear in reverse chronological order, while messages within each block are
in chronological order. This helps keep more relevant log-blocks near the top, while keeping a clean record of
the execution thread.

For demonstration (this is not the real syntax):
```logfile
[begin block 2]
message 1
message 2
[end block 2]
[begin block 1]
message 1
message 2
[end block 1]
```

## Block Syntax

Each block starts with an introducer in the following format:
`[beginblock <TIMESTAMP>:<ARGSTR>]`
Here, `<TIMESTAMP>` indicates when the program was invoked, and `<ARGSTR>` is a concetenation of `argv[1..]` to document
how catalyst was invoked.

Each block ends with:
`[endblock]`

Inside each block, messages are recorded.

> [!NOTE}
> If Catalyst detects that a git repo is being used, it furhter ammend the introducer to
> `[beginblock <TIMESTAMP>:<ARGSTR>:<git email>]` since blames aren't really effective.

## Message Syntax

Each message in a block follows the format:
```
<TIMESTAMP> <INFO | WARN | ERROR> <MESSAGE>
```
