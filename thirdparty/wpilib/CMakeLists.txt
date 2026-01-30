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
project(thirdparty-wpilib)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

# Fetch WPILib, configure it to only build CSCore and NTCore and their dependencies.
# This is needed because WPIMath uses a custom version of Eigen which is not compatible with the Eigen version used by GTSAM.
# Because this project is a lot more dependent on GTSAM than WPILib, we disable the portions of WPILib
# that depend on Eigen to avoid conflicts.
set(WITH_WPIMATH OFF)
set(WITH_WPILIB OFF)
set(WITH_TESTS OFF)
set(WITH_GUI OFF)
set(WITH_SIMULATION_MODULES OFF)
set(WITH_PROTOBUF OFF)
set(WITH_BENCHMARK OFF)
set(WPILIB_INSTALL OFF)
fetchcontent_declare(
    allwpilib
    GIT_REPOSITORY    https://github.com/wpilibsuite/allwpilib.git
    GIT_TAG           v2026.1.1
)
FetchContent_MakeAvailable(allwpilib)


