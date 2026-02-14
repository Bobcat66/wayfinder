/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2025 Jesse Kane
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <cstdint>
#include <array>
#include <opencv2/core.hpp>
#include <gtsam/geometry/Pose3.h>
#include <gtsam/linear/NoiseModel.h>
#include <gtsam/slam/expressions.h>

struct TagCornerObservation {
    uint64_t micros;
    int64_t server_time;
    int tag_id;
    std::array<cv::Point2f, 4> corners;
    gtsam::Cal3_S2_ cameraCal;
    // Offset from robot kinematic center -> camera optical center
    gtsam::Pose3 robotTcamera;
    // Pixel noise in camera
    gtsam::SharedNoiseModel cameraNoise;
};
