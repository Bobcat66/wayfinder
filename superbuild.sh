
# This script automatically performs a two-stage superbuild.
# First, it performs a bootstrap build which builds and installs
# Wayfinder's runtime dependencies. Then it runs the actual Wayfinder build.

# It allows the user to pass arbitrary cmake options to the main build
# Except for the source and build directories, and the build generator,
# which are hard-coded.
# it will build in build/custom

# Presets SHOULD NOT BE USED when using superbuild.sh

# It requires CMake (duh), and Ninja

# Stage 1: Bootstrapping
cmake --preset bootstrap
cmake --build --preset bootstrap

# Stage 2: Build
cmake -G Ninja -S . -B build/custom "$@"
cmake --build build/custom