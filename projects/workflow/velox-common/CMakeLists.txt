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
set(VELOX_SOURCES

    src/common/common.cpp  
    src/common/common.cpp  
    src/common/commands.cpp  
    src/common/commands.hpp  
    src/common/compiler_options.cpp
    src/common/compiler_options.hpp
    src/common/environment.cpp
    src/common/environment.hpp
    src/common/fileutils.cpp
    src/common/fileutils.hpp
    src/common/time.cpp
    src/common/time.hpp
    src/common/toolchain_options.cpp
    src/common/toolchain_options.hpp
    src/common/utils.cpp
    src/common/utils.hpp

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
