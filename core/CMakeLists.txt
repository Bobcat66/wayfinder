cmake_minimum_required(VERSION 3.25)
project(wayfindercore CXX C)

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)

file(GLOB_RECURSE CORESOURCES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/cpp/*.cpp"
)
add_library(wayfindercore ${CORESOURCES})

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_BASE REQUIRED gstreamer-base-1.0)

target_link_libraries(wayfindercore PRIVATE
    ${OpenCV_LIBS}
    ${GSTREAMER_LIBRARIES}
    ${GSTREAMER_BASE_LIBRARIES}
    gtsam
    ntcore
    cscore
    apriltag
    spdlog::spdlog
) # In deployment builds, this will link statically, and there will be a cmake build type variable set in the top-level CMake file. I'll put that in eventually. For now, this is nowhere near deployment

target_include_directories(wayfindercore 
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/include
    PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_BASE_INCLUDE_DIRS}
)

enable_testing()

file(GLOB_RECURSE TESTSOURCES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/test/native/*.cpp"
)
add_executable(run_tests ${TESTSOURCES})

target_link_libraries(run_tests wayfindercore)



