cmake_minimum_required(VERSION 3.25)
project(wayfinder VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# TODO: Fix dependency hierarchy. Right now the dependencies are a mess, eventually 
# I should configure the build system to make sure dependencies are imported once and only once
# Particular points i've identified as needing improvement are fmt and Eigen
# Also worried this stuff might break the ODR rule
# Ideally none of the dependencies would bundle any dependencies that are used by other dependencies, and everything
# would rely on a shared set of libraries bundled with this project

add_subdirectory(thirdparty/googletest)
add_subdirectory(thirdparty/apriltag)
add_subdirectory(thirdparty/wpilib)
add_subdirectory(thirdparty/gtsam)
add_subdirectory(thirdparty/spdlog)
add_subdirectory(wfcore)

