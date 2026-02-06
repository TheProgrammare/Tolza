cmake_minimum_required(VERSION 3.15)

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

# ----------------------------
# Sources
# ----------------------------
file(GLOB_RECURSE VELOX_SOURCES
    CONFIGURE_DEPENDS
    src/*.c
    src/*.cpp
)

add_executable(velox ${VELOX_SOURCES})

# ----------------------------
# Include directories
# ----------------------------
# Root for project includes
target_include_directories(velox
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# ----------------------------
# LLVM / Clang (portable way)
# ----------------------------
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

# LLVM include dirs & defs
target_include_directories(velox
    PRIVATE
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
)

target_compile_definitions(velox
    PRIVATE
        ${LLVM_DEFINITIONS}
)

# Link against LLVM + libclang
target_link_libraries(velox
    PRIVATE
        LLVM
        libclang
)

# ----------------------------
# Warnings (disabled by default)
# ----------------------------
if (MSVC)
    target_compile_options(velox PRIVATE /w)
else()
    target_compile_options(velox PRIVATE -w)
endif()

