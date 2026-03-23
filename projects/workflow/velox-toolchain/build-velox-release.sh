#!/bin/bash
# build_velox.sh
# Script to configure and build Velox using CMake + Ninja in parallel

set -e

# Build directory
NUM_THREADS=$(( $(nproc) * 3 / 2 ))
BUILD_DIR=build/release

# Create the build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

# Configure CMake with the Ninja generator and C++23 standard
cmake -G Ninja -DCMAKE_CXX_STANDARD=23 ../../

# Build in parallel using Ninja
echo "Building Velox with $NUM_THREADS threads..."
time ninja -j $NUM_THREADS velox-toolchain
