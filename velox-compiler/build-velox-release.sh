#!/bin/bash
# build_velox.sh
# Script to configure and build Velox using CMake + Ninja in parallel

set -e

# Determine the number of threads based on available CPU cores
NUM_CORES=$(nproc --all)

echo "Building with $JOBS threads..."

# Build directory
BUILD_DIR=build/release

# Create the build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

# Configure CMake with the Ninja generator and C++23 standard
cmake -G Ninja -DCMAKE_CXX_STANDARD=23 ../../

# Build in parallel using Ninja
echo "Building Velox with $NUM_CORES cores..."
time ninja -j $NUM_CORES
