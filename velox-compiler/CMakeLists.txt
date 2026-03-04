cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")


project(velox-compiler
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

add_executable(velox-compiler ${VELOX_SOURCES})

# ----------------------------
# Vendor
# ----------------------------
# Header-only vendor library
add_library(velox_vendor INTERFACE)

# ----------------------------
# Include directories
# ----------------------------
# Root for project includes
target_include_directories(velox-compiler
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_include_directories(velox_vendor
    INTERFACE
        ${CMAKE_SOURCE_DIR}/vendor
)

# ----------------------------
# LLVM / Clang (portable way)
# ----------------------------
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

# LLVM include dirs & defs
target_include_directories(velox-compiler
    PRIVATE
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
)

target_compile_definitions(velox-compiler
    PRIVATE
        ${LLVM_DEFINITIONS}
)

# Link against LLVM + libclang
target_link_libraries(velox-compiler
    PRIVATE
        LLVM
        libclang
        velox_vendor
)

