cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")



project(velox-toolchain
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

add_executable(velox-toolchain ${VELOX_SOURCES})

# ----------------------------
# Vendor
# ----------------------------
file(GLOB_RECURSE VENDOR_SOURCES
    CONFIGURE_DEPENDS
    vendor/*.cpp
    vendor/*.c
)

add_library(velox_vendor ${VENDOR_SOURCES})

# ----------------------------
# Include directories
# ----------------------------
# Root for project includes
target_include_directories(velox-toolchain
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_include_directories(velox_vendor
    SYSTEM PUBLIC
        ${CMAKE_SOURCE_DIR}/vendor
)

target_link_libraries(velox-toolchain
    PRIVATE
        velox_vendor
        velox-common
)

