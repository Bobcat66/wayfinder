# Building Wayfinder from Source

>[!WARNING]
>Building from source is a very involved process, and is generally not recommended for most use cases.

>[!IMPORTANT]
>Wayfinder is designed to be built with GCC 13 using the C++ 20 standard. Building with Clang and MSVC are not supported at the moment

>[!NOTE]
>This guide is designed for Ubuntu 24.04.1 LTS

## Installing prerequisites

To install the necessary prerequisites, run the following command in a terminal:
```
sudo apt update && apt install -y build-essential cmake ninja-build
```
After that, clone this repository and navigate into it:
```
git clone https://www.github.com/Bobcat66/wayfinder.git && cd wayfinder
```
After that, simply run
```
./tools/build.sh
```
This script will automatically run Wayfinder's two-stage build system, 
building the artifacts in the `build` directory. CMake arguments (except for presets and source and build directory arguments) can be passed directly to `tools/build.sh`