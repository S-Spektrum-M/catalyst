# catalyst test

```
test the project
Usage: catalyst test [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  --workspace                 Test all members in the workspace
  -P,--params TEXT ...        
```

## Examples

```bash
catalyst test
catalyst test --params "--gtest_filter=MyTest.*"
```

**Test workspace:**
Run tests for all members in the workspace.
```bash
catalyst test --workspace
```