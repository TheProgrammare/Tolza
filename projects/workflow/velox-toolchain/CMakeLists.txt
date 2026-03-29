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
file(GLOB_RECURSE VELOX_SOURCES
    CONFIGURE_DEPENDS
    src/*.cpp
)

# add velox-common scripts
# for the compiler data interface
add_subdirectory(../velox-common common_build)

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

