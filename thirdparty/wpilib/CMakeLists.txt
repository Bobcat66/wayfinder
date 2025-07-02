cmake_minimum_required(VERSION 3.25)
project(thirdparty-wpilib)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

# Fetch WPILib, configure it to only build CSCore and NTCore and their dependencies.
# This is needed because WPIMath uses a custom version of Eigen which is not compatible with the Eigen version used by GTSAM.
# Because this project is a lot more dependent on GTSAM than WPILib, we disable the portions of WPILib
# that depend on Eigen to avoid conflicts.
set(WITH_WPIMATH OFF)
set(WITH_WPILIB OFF)
set(WITH_TESTS OFF)
set(WITH_GUI OFF)
set(WITH_SIMULATION_MODULES OFF)
set(WITH_PROTOBUF OFF)
fetchcontent_declare(
    allwpilib
    GIT_REPOSITORY    https://github.com/wpilibsuite/allwpilib.git
    GIT_TAG           v2025.3.2
)
FetchContent_MakeAvailable(allwpilib)


