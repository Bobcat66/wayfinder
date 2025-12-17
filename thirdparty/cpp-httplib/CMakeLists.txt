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
project(thirdparty-cpp-httplib)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)
set(HTTPLIB_REQUIRE_ZSTD OFF)
set(HTTPLIB_USE_ZSTD_IF_AVAILABLE OFF)
fetchcontent_declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.25.0
)

#fetchcontent_populate(httplib)
#set(_httplib_dir "${CMAKE_BINARY_DIR}/_deps/httplib-src")
#set(_httplib_cmake "${_httplib_dir}/CMakeLists.txt")

#if (EXISTS "${_httplib_cmake}")
#    file(READ "${_httplib_cmake}" _content)
#    string(REPLACE
#        "AND \${CMAKE_SYSTEM_VERSION} VERSION_LESS"
#        "AND CMAKE_SYSTEM_VERSION VERSION_LESS"
#        _fixed "${_content}")
#    if (NOT _content STREQUAL _fixed)
#        file(WRITE "${_httplib_cmake}" "${_fixed}")
#        message(STATUS "Patched invalid Windows version check in httplib CMakeLists.txt")
#    endif()
#endif()

fetchcontent_populate(httplib)

add_library(httplib INTERFACE)

target_include_directories(httplib
    INTERFACE
        ${CMAKE_BINARY_DIR}/_deps/httplib-src
)

target_compile_definitions(httplib
    INTERFACE
)