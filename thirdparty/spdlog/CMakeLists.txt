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
project(thirdparty-spdlog)


include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

# Use compiled spdlog, not header-only
set(SPDLOG_HEADER_ONLY OFF CACHE BOOL "Build shared spdlog" FORCE)
set(SPDLOG_BUILD_SHARED ON CACHE BOOL "Build shared spdlog" FORCE)
set(SPDLOG_BUILD_STATIC OFF CACHE BOOL "" FORCE)
fetchcontent_declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.3
)
fetchcontent_makeavailable(spdlog)

# --- Install the built shared library ---
if(TARGET spdlog)
    # Ensure the library is built before installation
    install(TARGETS spdlog
        EXPORT spdlogTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )
else()
    message(FATAL_ERROR "spdlog target not found after FetchContent.")
endif()
