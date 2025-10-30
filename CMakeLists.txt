include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
)

FetchContent_MakeAvailable(yaml-cpp)

cmake_minimum_required(VERSION 3.10)
project(catalyst)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

if(CMAKE_C_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=mold")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Werror -Wextra -Wpedantic)

# Include directories
include_directories(include)

# --- Catalyst Executable ---
set(CATALYST_SOURCES
    src/catalyst.cpp
    src/hooks.cpp

    # YAML Utilities
    src/yaml-utils/Configuration.cpp
    src/yaml-utils/load_profile_file.cpp
    src/yaml-utils/profile_write_back.cpp

    # Add Subcommand
    src/subcommands/add/action.cpp
    src/subcommands/add/parse_cli.cpp

    # Build Subcommand
    src/subcommands/build/action.cpp
    src/subcommands/build/parse_cli.cpp

    # Clean Subcommand
    src/subcommands/clean/action.cpp
    src/subcommands/clean/parse_cli.cpp

    # Configure Subcommand
    src/subcommands/configure/action.cpp
    src/subcommands/configure/parse_cli.cpp

    # Fetch Subcommand
    src/subcommands/fetch/action.cpp
    src/subcommands/fetch/helper.cpp
    src/subcommands/fetch/parse_cli.cpp

    # Fmt Subcommand
    src/subcommands/fmt/action.cpp
    src/subcommands/fmt/parse_cli.cpp

    # Generate Subcommand
    src/subcommands/generate/action.cpp
    src/subcommands/generate/find_dep.cpp
    src/subcommands/generate/ld_path.cpp
    src/subcommands/generate/ldlibs.cpp
    src/subcommands/generate/parse_cli.cpp
    src/subcommands/generate/profile_comp.cpp
    src/subcommands/generate/source_set.cpp
    src/subcommands/generate/find_dep/git.cpp
    src/subcommands/generate/find_dep/local.cpp
    src/subcommands/generate/find_dep/system.cpp
    src/subcommands/generate/find_dep/vcpkg.cpp

    # Init Subcommand
    src/subcommands/init/action.cpp
    src/subcommands/init/parse_cli.cpp

    # Run Subcommand
    src/subcommands/run/action.cpp
    src/subcommands/run/parse_cli.cpp

    # Test Subcommand
    src/subcommands/test/action.cpp
    src/subcommands/test/parse_cli.cpp

    # Tidy Subcommand
    src/subcommands/tidy/action.cpp
    src/subcommands/tidy/parse_cli.cpp
)

# Create the executable from the source files
add_executable(catalyst ${CATALYST_SOURCES})

target_link_libraries(catalyst PRIVATE yaml-cpp::yaml-cpp)
find_package(CLI11 CONFIG REQUIRED)
target_link_libraries(catalyst PRIVATE CLI11::CLI11)


# Install the executable
install(TARGETS catalyst DESTINATION bin)
