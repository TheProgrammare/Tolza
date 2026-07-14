cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

project(velox
    VERSION 2026.1.1
    LANGUAGES C CXX
)

# ----------------------------
# Language standards
# ----------------------------
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# name
set(SOFTWARE_NAME "velox-toolchain")
# version
string(TIMESTAMP SOFTWARE_VERSION "%Y-%m-%d")

# ----------------------------
# Sources
# ----------------------------
set(VELOX_SOURCES

    src/command_package.hpp
    src/command_workspace.hpp
    src/command_audit.cpp
    src/cli_wrapper.hpp
    src/command_compiler.cpp
    src/command_compiler.hpp
    src/command_workspace.cpp
    src/toolchain/toolchain.hpp
    src/toolchain/main.cpp
    src/toolchain/parser_command.hpp
    src/toolchain/parser_command.cpp
    src/toolchain/toolchain.cpp
    src/command_audit.hpp
    src/command_build.cpp
    src/cli_wrapper.cpp
    src/command_check.hpp
    src/command_package.cpp
    src/command_check.cpp
    src/command_build.hpp

)

# add velox-common scripts
# for the compiler data interface
add_subdirectory(../velox-common velox-common)

add_executable(velox ${VELOX_SOURCES})

# ----------------------------
# Include directories
# ----------------------------
# Root for project includes
target_include_directories(velox
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(velox
    PRIVATE
        velox-common
)


# rename final binary
#set_target_properties(velox PROPERTIES
    #OUTPUT_NAME "${SOFTWARE_NAME}-${SOFTWARE_VERSION}b" # beta version mode
    #OUTPUT_NAME "${SOFTWARE_NAME}-${SOFTWARE_VERSION}" # normal version mode
#)

