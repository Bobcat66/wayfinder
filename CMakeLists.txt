cmake_minimum_required(VERSION 3.25)
project(wayfinder-all)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(thirdparty/apriltag)
add_subdirectory(thirdparty/wpilib)
add_subdirectory(thirdparty/gtsam)
add_subdirectory(core)

