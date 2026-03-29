cmake_minimum_required(VERSION 3.16)


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

add_library(velox-common ${VELOX_SOURCES})

# ----------------------------
# Include directories
# ----------------------------
# Root for project includes
target_include_directories(velox-common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor
)
