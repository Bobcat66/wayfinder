#!/bin/bash

# This script automatically performs a two-stage superbuild.
# First, it performs a bootstrap build which builds and installs
# Wayfinder's runtime dependencies. Then it runs the actual Wayfinder build.

# It automatically builds for release on aarch64 architecture, and places artifacts in the build_aarch64 directory

# It requires CMake, and Ninja

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STAGING_DIR="${SCRIPT_DIR}/../staging"

# Stage 1: Bootstrapping
echo "Bootstrapping build system..."
cmake --preset bootstrap-aarch64
cmake --build --preset bootstrap-aarch64

# Stage 2: stage bootstrapped extern deps
cmake -E env DESTDIR=$STAGING_DIR cmake --install "${SCRIPT_DIR}/../build_aarch64/extern_deps"

# Stage 3: Build
# Todo: Make this a preset? IDK
echo "Building Wayfinder..."
cmake -G Ninja -S . -B "${SCRIPT_DIR}/../build_aarch64/main" -DWF_INSTALL_BUILD=ON -DWF_BOOTSTRAP=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="${SCRIPT_DIR}/../cmake/toolchains/aarch64-gnu.cmake"
cmake --build "${SCRIPT_DIR}/../build_aarch64/main"

# Stage 4: Stage
echo "Staging Wayfinder..."
cmake -E env DESTDIR=$STAGING_DIR cmake --install "${SCRIPT_DIR}/../build_aarch64/main"
# Fix RPATHs in staged binaries and shared libraries
find "${SCRIPT_DIR}/../staging/opt/wayfinder/lib" -name '*.so*' -exec patchelf --set-rpath '$ORIGIN;$ORIGIN/../extern_deps/opencv/lib' {} \;
find "${SCRIPT_DIR}/../staging/opt/wayfinder/bin" -name 'wayfinderd' -exec patchelf --set-rpath '$ORIGIN/../lib;$ORIGIN/../extern_deps/opencv/lib' {} \;
