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
project(wayfinder-core CXX C)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)

file(GLOB_RECURSE CORESOURCES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/cpp/*.cpp"
)
file(GLOB_RECURSE WIPSSOURCES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/generated/wips/*.c"
)
file(GLOB_RECURSE JVALSOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/generated/jval/*.cpp"
)
add_library(wfcore ${CORESOURCES} ${WIPSSOURCES} ${JVALSOURCES})

install(TARGETS wfcore LIBRARY DESTINATION lib)

if(WF_USE_EDGETPU)
    add_compile_definitions(WF_USE_EDGETPU)
endif()

if(WF_USE_CUDA)
    add_compile_definitions(WF_USE_CUDA)
endif()

if(WF_USE_HAILO)
    add_compile_definitions(WF_USE_HAILO)
endif()

if(WF_USE_ROCM)
    add_compile_definitions(WF_USE_ROCM)
endif()

if(WF_USE_RKNN)
    add_compile_definitions(WF_USE_RKNN)
endif()

if(WF_USE_COREML)
    add_compile_definitions(WF_USE_COREML)
endif()

if(WF_USE_OPENVINO)
    add_compile_definitions(WF_USE_OPENVINO)
endif()

if(WF_USE_GSTREAMER)
    add_compile_definitions(WF_GSTREAMER)
    if(WF_USE_VIDEOLAN_CODECS)
        add_compile_definitions(WF_VIDEOLAN)
    endif()
endif()
# TODO: Figure out the library interface
target_link_libraries(wfcore 
    PUBLIC
    ${OpenCV_LIBS}
    gtsam
    spdlog::spdlog
    ntcore
    cscore
    nlohmann_json::nlohmann_json
    PRIVATE
    apriltag
)

target_include_directories(wfcore 
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/generated
    ${OpenCV_INCLUDE_DIRS}
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/internal
    ${CMAKE_CURRENT_BINARY_DIR}/../meta
)

if(WF_USE_GSTREAMER)
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
    pkg_check_modules(GSTREAMER_BASE REQUIRED gstreamer-base-1.0)

    target_link_libraries(wfcore
        PRIVATE
        ${GSTREAMER_LIBRARIES}
        ${GSTREAMER_BASE_LIBRARIES}
    )
    target_include_directories(wfcore
        PRIVATE
        ${GSTREAMER_INCLUDE_DIRS}
        ${GSTREAMER_BASE_INCLUDE_DIRS}
    )
    
endif()

if(WF_INSTALL_BUILD)
    message(STATUS "wfcore_tests will not run correctly with RPATH settings enabled, skipping test build")
else()
    file(GLOB_RECURSE TESTSOURCES 
        CONFIGURE_DEPENDS 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/test/native/*.cpp"
    )
    add_executable(wfcore_tests ${TESTSOURCES})
    target_link_libraries(wfcore_tests 
        PRIVATE
        GTest::gtest
        GTest::gtest_main
        wfcore
    )

    include(GoogleTest)
    gtest_discover_tests(wfcore_tests)
endif()



