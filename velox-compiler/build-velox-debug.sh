#!/bin/bash
# build_velox.sh
# Script to configure and build Velox using CMake + Ninja in parallel

set -e

# Build directory
BUILD_DIR=build/debug

# Create the build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

# Configure CMake with the Ninja generator and C++23 standard
#cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=23 -DCMAKE_CXX_FLAGS="-ftime-report" ../../
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=23 ../../

# Build in parallel using Ninja
echo "Building Velox with $(( $(nproc) * 3 / 2 )) threads..."
time ninja -j $(( $(nproc) * 3 / 2 ))