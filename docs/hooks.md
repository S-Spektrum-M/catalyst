# Hooks

Catalyst provides a powerful hook system that allows you to execute custom commands or scripts at various points in the build process. This enables you to extend and customize your build workflow to fit your project's specific needs.

## Configuring Hooks

You can define hooks in your `catalyst.yaml` or any profile-specific `catalyst_*.yaml` file. Hooks are defined under the `hooks` key. You can specify a single command or a list of commands to execute for each hook.

**Example `catalyst.yaml`:**

```yaml
hooks:
  pre_build: "echo 'Starting build...'"
  post_build:
    - command: "echo 'Build finished!'"
    - script: "scripts/notify.sh"
  on_build_failure: "echo 'Build failed!'"

```

## Available Hooks

Here is a comprehensive list of the available hooks in Catalyst, organized by when they are triggered.

### Global Hooks

These hooks are triggered for every build and provide a way to execute commands at the very beginning and end of the build process.

| Hook              | Description                                                                                             |
| ----------------- | ------------------------------------------------------------------------------------------------------- |
| `pre_build`       | Runs once before the entire build process begins (before `generate`, `fetch`, and `build`).               |
| `post_build`      | Runs once after the entire build process has successfully completed.                                    |
| `on_build_failure`| Runs if the build fails at any stage. This is useful for logging, sending notifications, or reverting changes. |


### Subcommand Hooks

These hooks are specific to individual Catalyst subcommands, allowing you to customize the behavior of each command.

#### `generate`

| Hook           | Description                                       |
| -------------- | ------------------------------------------------- |
| `pre_generate` | Runs before the `build.ninja` file is generated.  |
| `post_generate`| Runs after the `build.ninja` file is generated.   |


#### `fetch`

| Hook         | Description                                 |
| ------------ | ------------------------------------------- |
| `pre_fetch`  | Runs before any dependencies are fetched.   |
| `post_fetch` | Runs after all dependencies have been fetched.|


#### `clean`

| Hook        | Description                              |
| ----------- | ---------------------------------------- |
| `pre_clean` | Runs before the project is cleaned.      |
| `post_clean`| Runs after the project has been cleaned. |


#### `run`

| Hook      | Description                           |
| --------- | ------------------------------------- |
| `pre_run` | Runs before the target is executed.   |
| `post_run`| Runs after the target has been executed.|

#### `test`

| Hook       | Description                                |
| ---------- | ------------------------------------------ |
| `pre_test` | Runs before the tests are executed.        |
| `post_test`| Runs after the tests have been executed.   |


### Target-Specific Hooks

> [!NOTE]
> Not Yet Implemented.

These hooks can be defined within a specific build target's configuration, allowing for fine-grained control over the build process for individual targets.

| Hook        | Description                                                                 |
| ----------- | --------------------------------------------------------------------------- |
| `pre_link`  | Runs before the target is linked. This can be useful for pre-link steps.    |
| `post_link` | Runs after the target is linked. This can be useful for post-link steps.    |


### File-Specific Hooks

> [!NOTE]
> Not Yet Implemented.

These hooks are triggered when a specific file is processed, allowing for custom actions on a per-file basis.

| Hook         | Description                                                                                             |
| ------------ | ------------------------------------------------------------------------------------------------------- |
| `on_compile` | Runs when a specific source file is compiled. This can be useful for custom pre-processing or code generation. |
