
cmake_minimum_required(VERSION 3.25)
project(thirdparty-wpilib)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

# Fetch WPILib
set(WITH_WPIMATH OFF)
set(WITH_WPILIB OFF)
set(WITH_TESTS OFF)
set(WITH_GUI OFF)
set(WITH_SIMULATION_MODULES OFF)
fetchcontent_declare(
    allwpilib
    GIT_REPOSITORY    https://github.com/wpilibsuite/allwpilib.git
    GIT_TAG           1efaaefd78a634599b522fc2420c7283722554fb
)
fetchcontent_makeavailable(allwpilib)


