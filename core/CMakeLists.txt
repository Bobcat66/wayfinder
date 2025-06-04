cmake_minimum_required(VERSION 3.25)
project(wayfinder-core CXX C)

find_package(OpenCV REQUIRED)
find_package(GTSAM REQUIRED)

set(CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/native/cpp)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/native/*.cpp)
add_executable(wayfinder ${SOURCES})

target_link_libraries(wayfinder PRIVATE
    ${OpenCV_LIBS}
    gtsam
    ntcore
    cscore
) # In deployment builds, this will link statically, and there will be a cmake build type variable set in the top-level CMake file. I'll put that in eventually. For now, this is nowhere near deployment

target_include_directories(wayfinder
    PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${GTSAM_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/native/include
)
