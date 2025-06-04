cmake_minimum_required(VERSION 3.25)
project(thirdparty-apriltag)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

set(BUILD_PYTHON_WRAPPER OFF)
set(BUILD_EXAMPLES OFF)
set(BUILD_TESTING OFF)
fetchcontent_declare(
    apriltag
    GIT_REPOSITORY    https://github.com/AprilRobotics/apriltag.git
    GIT_TAG           v3.4.3
)
fetchcontent_makeavailable(apriltag)