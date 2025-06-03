# Copyright (c) 2018-2024 FIRST and other WPILib contributors
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# Neither the name of FIRST, WPILib, nor the names of other WPILib
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY FIRST AND OTHER WPILIB CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY NONINFRINGEMENT AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL FIRST OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "" FORCE)
endif()

cmake_minimum_required(VERSION 3.25)
set(CMAKE_OSX_ARCHITECTURES arm64 x86_64)
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT Embedded)
project(thirdparty-gtsam)

include(FetchContent)

# Fetch Eigen
set(EIGEN_BUILD_BLAS OFF)
set(EIGEN_BUILD_LAPACK OFF)
set(EIGEN_BUILD_CMAKE_PACKAGE ON)
fetchcontent_declare(
    Eigen3
    GIT_REPOSITORY    https://gitlab.com/libeigen/eigen.git
    GIT_TAG           0fb2ed140d4fc0108553ecfb25f2d7fc1a9319a1
    SYSTEM # Ignore warnings
    # Make gtsam pick up this Eigen as our system Eigen
    OVERRIDE_FIND_PACKAGE
)

# And build GTSAM
set(GTSAM_COMPILE_OPTIONS_PRIVATE_DEBUG CACHE INTERNAL FORCE)
set(GTSAM_COMPILE_OPTIONS_PRIVATE_RELWITHDEBINFO CACHE INTERNAL FORCE)
set(GTSAM_ENABLE_BOOST_SERIALIZATION OFF)
set(GTSAM_USE_BOOST_FEATURES OFF)
set(GTSAM_BUILD_PYTHON OFF)
set(GTSAM_BUILD_EXAMPLES_ALWAYS OFF)
set(GTSAM_BUILD_TESTS OFF)
set(BUILD_SHARED_LIBS OFF)
set(GTSAM_USE_SYSTEM_EIGEN ON)
if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-use-after-free")
endif()
fetchcontent_declare(
    gtsam
    GIT_REPOSITORY    https://github.com/borglab/gtsam
    GIT_TAG           42cc0ac922460d506a7bbf3a37a4a0b1e103b79e
)
fetchcontent_makeavailable(gtsam)