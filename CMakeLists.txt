cmake_minimum_required(VERSION 3.15)

project(velox
    VERSION 120261
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Gather all sources
file(GLOB_RECURSE PROJECT_SOURCES src/*.cpp)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)

# Create executable
add_executable(velox ${PROJECT_SOURCES})

# Include LLVM headers
target_include_directories(velox
    PRIVATE
        ${LLVM_INCLUDE_DIRS}
)

# Compile definitions from LLVM
target_compile_definitions(velox
    PRIVATE
        ${LLVM_DEFINITIONS}
)

# Disable warnings if desired
target_compile_options(velox PRIVATE )

# ---- Portable linking using llvm-config ----

# Get the required compiler flags
execute_process(
    COMMAND llvm-config --cxxflags
    OUTPUT_VARIABLE LLVM_CXXFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE " " ";" LLVM_CXXFLAGS_LIST "${LLVM_CXXFLAGS}")
target_compile_options(velox PRIVATE ${LLVM_CXXFLAGS_LIST})

# Get all libraries required for LLVM
execute_process(
    COMMAND llvm-config --libs all
    OUTPUT_VARIABLE LLVM_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE " " ";" LLVM_LIBS_LIST "${LLVM_LIBS}")
target_link_libraries(velox PRIVATE ${LLVM_LIBS_LIST})

# Get libclang library specifically
execute_process(
    COMMAND llvm-config --libs clang
    OUTPUT_VARIABLE LLVM_LIBCLANG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE " " ";" LLVM_LIBCLANG_LIST "${LLVM_LIBCLANG}")
target_link_libraries(velox PRIVATE ${LLVM_LIBCLANG_LIST})
