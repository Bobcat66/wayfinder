cmake_minimum_required(VERSION 3.10)
project(slamvision-core CXX C)

find_package(OpenCV REQUIRED)
find_package(GTSAM REQUIRED)

add_library(slamvision src/native/cpp/main.cpp)

target_link_libraries(slamvision ${OpenCV_LIBS} gtsam)

target_include_directories(slamvision PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${GTSAM_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/native/include
)
