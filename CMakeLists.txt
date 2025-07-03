cmake_minimum_required(VERSION 3.25)
project(wayfinder VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

# TODO: Fix dependency hierarchy. Right now the dependencies are a mess, eventually 
# I should configure the build system to make sure dependencies are imported once and only once
# Particular points i've identified as needing improvement are fmt and Eigen
# Also worried this stuff might break the ODR rule
# Ideally none of the dependencies would bundle any dependencies that are used by other dependencies, and everything
# would rely on a shared set of libraries bundled with this project
# Additionally, build OpenCV from source instead of relying on system installs

# This will build wayfinder's dependencies, but not wayfinder itself.
# The build system must be bootstrapped AT LEAST ONCE before building. Subsequent builds can be run without bootstrapping
option(WF_BOOTSTRAP "Enable bootstrap build" OFF)
option(WF_USE_GSTREAMER "Enable Gstreamer video capture" OFF) #WIP, low priority
option(WF_USE_VIDEOLAN_CODECS "Enable support for VideoLAN x264 and x265 codecs" OFF) #WIP, low priority
option(WF_BUILD_DAEMON "Builds a Wayfinder daemon meant to run on a coprocessor" ON)
option(WF_BUILD_CLIENT "Builds a Wayfinder desktop client, meant to both provide an interface to wayfinder, and to act as a SLAM server" OFF)

option(WF_USE_EDGETPU "Enable support for Google Edge TPU hardware-accelerated inference" OFF) #WIP, high priority
option(WF_USE_CUDA "Enable support for Nvidia CUDA hardware-accelerated inference" OFF) #WIP, high priority
option(WF_USE_HAILO "Enable support for Hailo-8 hardware-accelerated inference" OFF) #WIP, medium priority
option(WF_USE_ROCM "Enable support for AMD ROCm hardware-accelerated inference" OFF) #WIP, low priority
option(WF_USE_RKNN "Enable support for RKNPU hardware-accelerated inference" OFF) #WIP, high priority
option(WF_USE_COREML "Enable support for Apple CoreML hardware-accelerated inference" OFF) #WIP, low priority
option(WF_USE_OPENVINO "Enable support for Intel OpenVINO hardware-accelerated inference" OFF) #WIP, low priority

option(WF_USE_SYSTEM_OPENCV "Use system OpenCV instead of bundled version" OFF)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/meta/version.h
    @ONLY
)

if (WF_BOOTSTRAP)
    add_subdirectory(thirdparty/opencv)
else()
    add_subdirectory(thirdparty/opencv)
    add_subdirectory(thirdparty/googletest)
    add_subdirectory(thirdparty/apriltag)
    add_subdirectory(thirdparty/wpilib)
    add_subdirectory(thirdparty/gtsam)
    add_subdirectory(thirdparty/spdlog)
    add_subdirectory(thirdparty/nlohmann_json)
    add_subdirectory(core)
    add_subdirectory(daemon)
endif()

