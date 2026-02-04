cmake_minimum_required(VERSION 3.15)

project(velox
    VERSION 120261
    LANGUAGES CXX
)

add_compile_options(-w)

# =========================
# Configuration C++
# =========================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =========================
# Sources (recursive)
# =========================
file(GLOB_RECURSE PROJECT_SOURCES
    src/*.cpp
    src/*.cc
    src/*.cxx
)

if (PROJECT_SOURCES STREQUAL "")
    message(FATAL_ERROR "None source file found in src/")
endif()

# =========================
# LLVM
# =========================
find_program(LLVM_CONFIG_EXECUTABLE llvm-config REQUIRED)

execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --cxxflags
    OUTPUT_VARIABLE LLVM_CXXFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
    OUTPUT_VARIABLE LLVM_LDFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs
    OUTPUT_VARIABLE LLVM_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --system-libs
    OUTPUT_VARIABLE LLVM_SYSTEM_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --includedir
    OUTPUT_VARIABLE LLVM_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# =========================
# Target
# =========================
add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
)
# =========================
# Clang / libclang
# =========================
target_include_directories(velox
    PRIVATE
        src
        /usr/include
        /usr/lib/llvm/include
)

# Explicit link
target_link_libraries(velox
    PRIVATE
        clang
)


target_include_directories(${PROJECT_NAME}
    PRIVATE
        src
        ${LLVM_INCLUDE_DIR}
        ${CLANG_INCLUDE_DIR}
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        ${LLVM_CXXFLAGS}
)

target_link_options(${PROJECT_NAME}
    PRIVATE
        ${LLVM_LDFLAGS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${LLVM_LIBS}
        ${LLVM_SYSTEM_LIBS}
        ${CLANG_LIBRARY}
)

add_compile_options(-w)

# =========================
# Debug infos
# =========================
message(STATUS "LLVM include dir : ${LLVM_INCLUDE_DIR}")
message(STATUS "Clang include dir: ${CLANG_INCLUDE_DIR}")
message(STATUS "Clang library    : ${CLANG_LIBRARY}")
