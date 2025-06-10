cmake_minimum_required(VERSION 3.25)
project(wayfinder-core CXX C)

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/main/native/cpp/*.cpp)
add_executable(wayfinder ${SOURCES})

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_BASE REQUIRED gstreamer-base-1.0)

target_link_libraries(wayfinder PRIVATE
    ${OpenCV_LIBS}
    ${GSTREAMER_LIBRARIES}
    ${GSTREAMER_BASE_LIBRARIES}
    gtsam
    ntcore
    cscore
    apriltag
    spdlog::spdlog
) # In deployment builds, this will link statically, and there will be a cmake build type variable set in the top-level CMake file. I'll put that in eventually. For now, this is nowhere near deployment

target_include_directories(wayfinder
    PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_BASE_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/include
)
