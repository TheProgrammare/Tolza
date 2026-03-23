cmake_minimum_required(VERSION 3.16)

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
)
