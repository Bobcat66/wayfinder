# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (C) 2025 Jesse Kane
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

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
                -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
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
        set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/staging/${OPENCV_INSTALL_DIR}/lib/cmake/opencv4" PARENT_SCOPE)
        message("Using bundled OpenCV from: ${OpenCV_DIR}")
    endif()
endif()