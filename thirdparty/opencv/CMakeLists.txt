cmake_minimum_required(VERSION 3.25)
project(thirdparty-opencv)

if(WF_BOOTSTRAP)
    include(ExternalProject)
    set(OPENCV_INSTALL_DIR ${CMAKE_BINARY_DIR}/thirdparty/opencv-install)

    ExternalProject_Add(opencv
        GIT_REPOSITORY https://github.com/opencv/opencv.git
        GIT_TAG 4.11.0
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${OPENCV_INSTALL_DIR}
        -DBUILD_SHARED_LIBS=ON
        -DBUILD_EXAMPLES=OFF
        -DBUILD_TESTS=OFF
        -DBUILD_PERF_TESTS=OFF
        -DBUILD_JAVA=ON
        -DBUILD_opencv_python2=OFF
        -DBUILD_opencv_python3=OFF
    )
else()
    # Normal project build
    # Tell CMake where to find OpenCV if installed
    set(OPENCV_INSTALL_DIR ${CMAKE_BINARY_DIR}/thirdparty/opencv-install)
    set(CMAKE_PREFIX_PATH ${OPENCV_INSTALL_DIR})
endif()