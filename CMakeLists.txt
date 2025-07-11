cmake_minimum_required(VERSION 3.10)
project(catalyst)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

if(CMAKE_C_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=mold")
endif()

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Werror -Wextra -Wpedantic)

# Include directories
include_directories(include)

# Gather source files
file(GLOB_RECURSE ACTION_SOURCES "src/subcommands/*/action.cpp")

file(GLOB_RECURSE PARSE_CLI_SOURCES "src/subcommands/*/parse_cli.cpp")

file(GLOB_RECURSE YAML_UTILS_SOURCES "src/yaml-utils/*.cpp")

add_executable(catalyst
    src/catalyst.cpp
    ${ACTION_SOURCES}
    ${PARSE_CLI_SOURCES}
    ${YAML_UTILS_SOURCES}
)

include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
)

FetchContent_MakeAvailable(yaml-cpp)

target_link_libraries(catalyst PRIVATE yaml-cpp::yaml-cpp)

# Install the executable
install(TARGETS catalyst DESTINATION bin)
