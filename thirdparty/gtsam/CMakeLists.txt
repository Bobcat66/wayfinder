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
project(thirdparty-gtsam)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

set(GTSAM_BUILD_PYTHON OFF)
set(GTSAM_BUILD_UNSTABLE OFF)
set(GTSAM_ENABLE_BOOST_SERIALIZATION OFF)
set(GTSAM_USE_BOOST_FEATURES OFF)
set(GTSAM_BUILD_CONVENIENCE_LIBRARIES OFF)
set(GTSAM_BUILD_TESTS OFF)
set(GTSAM_BUILD_EXAMPLES_ALWAYS OFF)
set(GTSAM_BUILD_DOCS OFF)
fetchcontent_declare(
    gtsam
    GIT_REPOSITORY    https://github.com/borglab/gtsam.git
    GIT_TAG           4.3a0
)
fetchcontent_makeavailable(gtsam)
foreach(tgt gtsam gtsam_shared gtsam_static)
  if(TARGET ${tgt})
    target_compile_options(${tgt} PRIVATE
      $<$<COMPILE_LANGUAGE:CXX>:-Wno-overloaded-virtual>
      $<$<COMPILE_LANGUAGE:CXX>:-Wno-error>
    )
  endif()
endforeach()
