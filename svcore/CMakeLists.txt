cmake_minimum_required(VERSION 3.10)
project(svcore CXX C)

find_package(OpenCV REQUIRED)
find_package(GTSAM REQUIRED)

add_executable(svcore src/native/cpp/main.cpp)

target_link_libraries(svcore PRIVATE
    ${OpenCV_LIBS}
    gtsam
    ntcore
    cscore
)

target_include_directories(svcore 
    PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${GTSAM_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/native/include
)
