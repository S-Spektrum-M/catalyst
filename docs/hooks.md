# Hooks

Catalyst provides a powerful hook system that allows you to execute custom commands or scripts at various points in the build process. This enables you to extend and customize your build workflow to fit your project's specific needs.

## Configuring Hooks

You can define hooks in your `catalyst.yaml` or any profile-specific `catalyst_*.yaml` file. Hooks are defined under the `hooks` key, and you can specify a list of commands or scripts to execute for each hook.

**Example `catalyst.yaml`:**

```yaml
hooks:
  pre-build:
    - command: "echo 'Starting build...'"
    - script: "scripts/prepare-build.sh"
  post-build:
    - command: "echo 'Build finished!'"
```

## Available Hooks

Here is a comprehensive list of the available hooks in Catalyst, organized by when they are triggered.

### Global Hooks

These hooks are triggered for every build and provide a way to execute commands at the very beginning and end of the build process.

| Hook              | Description                                                                                             |
| ----------------- | ------------------------------------------------------------------------------------------------------- |
| `pre-build`       | Runs once before the entire build process begins (before `generate`, `fetch`, and `build`).               |
| `post-build`      | Runs once after the entire build process has successfully completed.                                    |
| `on-build-failure`| Runs if the build fails at any stage. This is useful for logging, sending notifications, or reverting changes. |


### Subcommand Hooks

These hooks are specific to individual Catalyst subcommands, allowing you to customize the behavior of each command.

#### `build`

| Hook         | Description                               |
| ------------ | ----------------------------------------- |
| `pre-build`  | Runs before the `build` command starts.   |
| `post-build` | Runs after the `build` command finishes.  |


#### `generate`

| Hook           | Description                                       |
| -------------- | ------------------------------------------------- |
| `pre-generate` | Runs before the `build.ninja` file is generated.  |
| `post-generate`| Runs after the `build.ninja` file is generated.   |


#### `fetch`

| Hook         | Description                                 |
| ------------ | ------------------------------------------- |
| `pre-fetch`  | Runs before any dependencies are fetched.   |
| `post-fetch` | Runs after all dependencies have been fetched.|


#### `clean`

| Hook        | Description                              |
| ----------- | ---------------------------------------- |
| `pre-clean` | Runs before the project is cleaned.      |
| `post-clean`| Runs after the project has been cleaned. |


#### `run`

| Hook      | Description                           |
| --------- | ------------------------------------- |
| `pre-run` | Runs before the target is executed.   |
| `post-run`| Runs after the target has been executed.|


### Target-Specific Hooks

These hooks can be defined within a specific build target's configuration, allowing for fine-grained control over the build process for individual targets.

| Hook        | Description                                                                 |
| ----------- | --------------------------------------------------------------------------- |
| `pre-link`  | Runs before the target is linked. This can be useful for pre-link steps.    |
| `post-link` | Runs after the target is linked. This can be useful for post-link steps.    |


### File-Specific Hooks
These hooks are triggered when a specific file is processed, allowing for custom actions on a per-file basis.

| Hook         | Description                                                                                             |
| ------------ | ------------------------------------------------------------------------------------------------------- |
| `on-compile` | Runs when a specific source file is compiled. This can be useful for custom pre-processing or code generation. |
