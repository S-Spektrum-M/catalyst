#include "catalyst/subcommands/init.hpp"
#include "catalyst/log-utils/log.hpp"

#include <filesystem>
#include <fstream>
#include <format>
#include <string>

namespace catalyst::init {

namespace fs = std::filesystem;

template <> std::expected<void, std::string> emitIDEConfig<Parse::IdeType::vsc>(const Parse& parse_args) {
    catalyst::logger.log(LogLevel::INFO, "Generating VS Code IDE configuration");

    fs::path vscode_dir = parse_args.path / ".vscode";
    if (!fs::exists(vscode_dir)) {
        fs::create_directories(vscode_dir);
    }

    // c_cpp_properties.json
    {
        std::ofstream cpp_props(vscode_dir / "c_cpp_properties.json");
        if (!cpp_props) return std::unexpected("Failed to create .vscode/c_cpp_properties.json");
        cpp_props << R"json({
    "configurations": [
        {
            "name": "Catalyst",
            "compileCommands": "${workspaceFolder}/build/compile_commands.json",
            "intelliSenseMode": "linux-clang-x64"
        }
    ],
    "version": 4
}
)json";
    }

    // tasks.json
    {
        std::ofstream tasks(vscode_dir / "tasks.json");
        if (!tasks) return std::unexpected("Failed to create .vscode/tasks.json");
        tasks << R"json({
    "version": "2.0.0",
    "tasks": [
        {
            "label": "catalyst build",
            "type": "shell",
            "command": "catalyst build",
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "catalyst run",
            "type": "shell",
            "command": "catalyst run",
            "problemMatcher": []
        },
        {
            "label": "catalyst test",
            "type": "shell",
            "command": "catalyst test",
            "group": "test",
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "catalyst clean",
            "type": "shell",
            "command": "catalyst clean",
            "problemMatcher": []
        }
    ]
}
)json";
    }

    // launch.json
    {
        std::ofstream launch(vscode_dir / "launch.json");
        if (!launch) return std::unexpected("Failed to create .vscode/launch.json");
        launch << std::format(R"json({{
    "version": "0.2.0",
    "configurations": [
        {{
            "name": "Debug (Catalyst)",
            "type": "cppdbg",
            "request": "launch",
            "program": "${{workspaceFolder}}/build/{}",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${{workspaceFolder}}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "preLaunchTask": "catalyst build"
        }}
    ]
}}
)json", parse_args.name);
    }

    // settings.json
    {
        std::ofstream settings(vscode_dir / "settings.json");
        if (!settings) return std::unexpected("Failed to create .vscode/settings.json");
        settings << R"json({
    "C_Cpp.default.compileCommands": "${workspaceFolder}/build/compile_commands.json",
    "C_Cpp.formatting": "clangFormat",
    "editor.formatOnSave": true
}
)json";
    }

    return {};
}

template <> std::expected<void, std::string> emitIDEConfig<Parse::IdeType::clion>(const Parse& parse_args) {
    catalyst::logger.log(LogLevel::INFO, "Generating CLion IDE configuration");

    fs::path idea_dir = parse_args.path / ".idea";
    fs::path run_configs_dir = idea_dir / "runConfigurations";
    fs::path tools_dir = idea_dir / "tools";

    if (!fs::exists(run_configs_dir)) {
        fs::create_directories(run_configs_dir);
    }
    if (!fs::exists(tools_dir)) {
        fs::create_directories(tools_dir);
    }

    // External Tools
    {
        std::ofstream tools_xml(tools_dir / "External Tools.xml");
        if (!tools_xml) return std::unexpected("Failed to create .idea/tools/External Tools.xml");
        tools_xml << R"xml(<toolSet name="External Tools">
  <tool name="catalyst build" description="Build project with Catalyst" showInMainMenu="false" showInEditor="false" showInProject="false" showInSearchPopup="false" disabled="false" useConsole="true" showConsoleOnStdOut="false" showConsoleOnStdErr="false" synchronizeAfterRun="true">
    <exec>
      <option name="COMMAND" value="catalyst" />
      <option name="PARAMETERS" value="build" />
      <option name="WORKING_DIRECTORY" value="$ProjectFileDir$" />
    </exec>
  </tool>
  <tool name="catalyst clean" description="Clean project with Catalyst" showInMainMenu="false" showInEditor="false" showInProject="false" showInSearchPopup="false" disabled="false" useConsole="true" showConsoleOnStdOut="false" showConsoleOnStdErr="false" synchronizeAfterRun="true">
    <exec>
      <option name="COMMAND" value="catalyst" />
      <option name="PARAMETERS" value="clean" />
      <option name="WORKING_DIRECTORY" value="$ProjectFileDir$" />
    </exec>
  </tool>
</toolSet>
)xml";
    }

    // Run Configuration
    {
        std::ofstream run_xml(run_configs_dir / "Catalyst.xml");
        if (!run_xml) return std::unexpected("Failed to create .idea/runConfigurations/Catalyst.xml");
        run_xml << std::format(R"xml(<component name="ProjectRunConfigurationManager">
  <configuration default="false" name="Catalyst Run/Debug" type="CLionNativeAppRunConfigurationType" REDIRECT_INPUT="false" ELEVATE="false" USE_EXTERNAL_CONSOLE="false" PASS_PARENT_ENVS_2="true" PROJECT_NAME="{0}" TARGET_NAME="{0}" CONFIG_NAME="{0}" version="1" RUN_PATH="$PROJECT_DIR$/build/{0}">
    <method v="2">
      <option name="ToolBeforeRunTask" enabled="true" actionId="Tool_External Tools_catalyst build" />
    </method>
  </configuration>
</component>
)xml", parse_args.name);
    }

    return {};
}

} // namespace catalyst::init
