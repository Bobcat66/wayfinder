#!/bin/bash

# This script automatically performs a two-stage superbuild.
# First, it performs a bootstrap build which builds and installs
# Wayfinder's runtime dependencies. Then it runs the actual Wayfinder build.

# It allows the user to pass arbitrary cmake options to the main build
# Except for the source and build directories, and the build generator,
# which are hard-coded.
# it will build in build/custom

# Presets SHOULD NOT BE USED when using build.sh

# It requires CMake, and Ninja

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STAGING_DIR="${SCRIPT_DIR}/../staging"

# Stage 1: Bootstrapping
echo "Bootstrapping build system..."
cmake --preset bootstrap
cmake --build --preset bootstrap

# Stage 2: Build
echo "Building Wayfinder..."
cmake -G Ninja -S . -B "${SCRIPT_DIR}/../build/main" -DWF_BOOTSTRAP=OFF "$@"
cmake --build "${SCRIPT_DIR}/../build/main"

# Stage 3: Stage
echo "Staging Wayfinder..."
cmake -E env DESTDIR=$STAGING_DIR cmake --install "${SCRIPT_DIR}/../build/main"