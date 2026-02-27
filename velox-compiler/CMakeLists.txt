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

# ----------------------------
# Sources
# ----------------------------
file(GLOB_RECURSE VELOX_SOURCES
    CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(velox ${VELOX_SOURCES})

add_compile_definitions(k_project_dir="${CMAKE_SOURCE_DIR}")

option(VELOX_DEV_ARGS "Enable dev CLI args" ON)

if(VELOX_DEV_ARGS)
  target_compile_definitions(velox PRIVATE VELOX_DEV_ARGS)
endif()

# ----------------------------
# Thirdparty
# ----------------------------
file(GLOB_RECURSE THIRD_SOURCES
    CONFIGURE_DEPENDS
    thirdparty/*.cpp
    thirdparty/*.c
)

add_library(velox_thirdparty ${THIRD_SOURCES})

# ----------------------------
# Include directories
# ----------------------------
# Root for project includes
target_include_directories(velox
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_include_directories(velox_thirdparty
    SYSTEM PUBLIC
        ${CMAKE_SOURCE_DIR}/thirdparty
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
        velox_thirdparty
)

# ----------------------------
# Warnings (disabled by default)
# ----------------------------

#if (MSVC)
#    target_compile_options(velox PRIVATE /w)
#else()
#    target_compile_options(velox PRIVATE -w)
#endif()