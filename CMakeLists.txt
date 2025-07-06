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

# Include directories
include_directories(include)

# Gather source files
file(GLOB_RECURSE ACTION_SOURCES
    "src/subcommands/add/action.cpp"
    "src/subcommands/init/action.cpp"
)
file(GLOB_RECURSE PARSE_CLI_SOURCES
    "src/subcommands/add/parse_cli.cpp"
    "src/subcommands/init/parse_cli.cpp"
)
file(GLOB_RECURSE YAML_UTILS_SOURCES "src/yaml-utils/*.cpp")

# Create static libraries
add_library(catalyst_act STATIC ${ACTION_SOURCES})
add_library(catalyst_parse STATIC ${PARSE_CLI_SOURCES})
add_library(yaml_utils STATIC ${YAML_UTILS_SOURCES})

# Add the executable
add_executable(catalyst src/catalyst.cpp)

# Link libraries
target_link_libraries(catalyst PRIVATE catalyst_act catalyst_parse yaml_utils)

# Find and link yaml-cpp
find_package(yaml-cpp REQUIRED)
target_link_libraries(catalyst PRIVATE yaml-cpp)

# Install the executable
install(TARGETS catalyst DESTINATION bin)
