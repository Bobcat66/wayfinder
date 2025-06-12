cmake_minimum_required(VERSION 3.25)
project(wayfinder-core CXX C)

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)

file(GLOB_RECURSE CORESOURCES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/cpp/*.cpp"
)
add_library(wfcore ${CORESOURCES})

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_BASE REQUIRED gstreamer-base-1.0)

# TODO: Figure out the library interface
target_link_libraries(wfcore 
    PUBLIC
    gtsam
    ${OpenCV_LIBS}
    spdlog::spdlog
    PRIVATE
    ntcore
    cscore
    apriltag
    SHARED ${GSTREAMER_LIBRARIES}
    SHARED ${GSTREAMER_BASE_LIBRARIES} #GStreamer, being LGPL-licensed, cannot be statically linked to this project
) # In deployment builds, this will link statically, and there will be a cmake build type variable set in the top-level CMake file. I'll put that in eventually. For now, this is nowhere near deployment

target_include_directories(wfcore 
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/include
    ${OpenCV_INCLUDE_DIRS}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/internal_headers
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_BASE_INCLUDE_DIRS}
)

enable_testing()

file(GLOB_RECURSE TESTSOURCES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/test/native/*.cpp"
)
add_executable(run_tests ${TESTSOURCES})

target_link_libraries(run_tests 
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    wfcore
)



