cmake_minimum_required(VERSION 3.15)

project(velox
    VERSION 120261
    LANGUAGES C CXX
)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE PROJECT_SOURCES
    src/*.cpp
)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

add_executable(velox ${PROJECT_SOURCES})

target_include_directories(velox
    PRIVATE
        src
        ${LLVM_INCLUDE_DIRS}
)

target_compile_definitions(velox
    PRIVATE
        ${LLVM_DEFINITIONS}
)

target_link_libraries(velox
    PRIVATE
        clang-cpp
)


target_compile_options(velox
    PRIVATE
        -w
)
