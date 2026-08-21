cmake_minimum_required(VERSION 3.28)

project(tolza
    VERSION 2026.8
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


# ----------------------------
# Sources
# ----------------------------
set(TOLZA_SOURCES

    ./src/cli_wrapper.cpp
    ./src/cli_wrapper.hpp
    ./src/command_audit.cpp
    ./src/command_audit.hpp
    ./src/command_build.cpp
    ./src/command_build.hpp
    ./src/command_check.cpp
    ./src/command_check.hpp
    ./src/command_compiler.cpp
    ./src/command_compiler.hpp
    ./src/command_package.cpp
    ./src/command_package.hpp
    ./src/command_workspace.cpp
    ./src/command_workspace.hpp
    ./src/toolchain/main.cpp
    ./src/toolchain/parser_command.cpp
    ./src/toolchain/parser_command.hpp
    ./src/toolchain/toolchain.cpp
    ./src/toolchain/toolchain.hpp

)

# add tolza-common scripts
# for the compiler data interface
add_subdirectory(../common tolza-common)


add_executable(tolza ${TOLZA_SOURCES})

# version
string(TIMESTAMP SOFTWARE_VERSION "%Y-%m-%d")
string(TIMESTAMP TOLZA_VERSION "%Y-%m")

target_compile_definitions(tolza
    PUBLIC
        SOFTWARE_NAME="tolza-toolchain"
        SOFTWARE_VERSION="${SOFTWARE_VERSION}"
        TOLZA_VERSION="${TOLZA_VERSION}"
)

# ----------------------------
# Include directories
# ----------------------------
# Root for project includes
target_include_directories(tolza
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(tolza
    PRIVATE
        tolza-common
)


# rename final binary
#set_target_properties(tolza PROPERTIES
    #OUTPUT_NAME "${SOFTWARE_NAME}-${SOFTWARE_VERSION}b" # beta version mode
    #OUTPUT_NAME "${SOFTWARE_NAME}-${SOFTWARE_VERSION}" # normal version mode
#)

include(GNUInstallDirs)

install(
    TARGETS tolza
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
