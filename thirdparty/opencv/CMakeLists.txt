cmake_minimum_required(VERSION 3.25)
project(thirdparty-opencv)

if(NOT WF_USE_SYSTEM_OPENCV)
    # Not using system OpenCV, use the bundled version
    if(WF_BOOTSTRAP)
        #Bootstrap build, build OpenCV
        include(ExternalProject)
        set(OPENCV_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/extern_deps/opencv)
        
        ExternalProject_Add(
            opencv
            GIT_REPOSITORY https://github.com/opencv/opencv.git
            GIT_TAG 4.11.0
            CMAKE_ARGS
                -DCMAKE_INSTALL_PREFIX=${OPENCV_INSTALL_DIR}
                -DCMAKE_BUILD_TYPE=Release
                -DBUILD_SHARED_LIBS=ON
                -DBUILD_EXAMPLES=OFF
                -DBUILD_TESTS=OFF
                -DBUILD_PERF_TESTS=OFF
                -DBUILD_JAVA=ON
                -DBUILD_opencv_python2=OFF
                -DBUILD_opencv_python3=OFF
            LOG_DOWNLOAD ON
            LOG_CONFIGURE ON
            LOG_BUILD ON
            LOG_INSTALL ON
            BUILD_COMMAND ${CMAKE_COMMAND} --build . --verbose
            INSTALL_COMMAND mkdir -p ${CMAKE_SOURCE_DIR}/staging
            COMMAND ${CMAKE_COMMAND} -E env DESTDIR=${CMAKE_SOURCE_DIR}/staging ${CMAKE_COMMAND} --build . --target install
        )
    else()
        # Normal project build
        # Tell CMake where to find OpenCV if installed
        set(OPENCV_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/extern_deps/opencv)
        set(OpenCV_DIR "${OPENCV_SOURCE_DIR}/staging/${OPENCV_INSTALL_DIR}/lib/cmake/opencv4" PARENT_SCOPE)
    endif()
endif()