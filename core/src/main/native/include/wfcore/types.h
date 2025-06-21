/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of Wayfinder.
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

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <gtsam/geometry/Pose3.h>
#include <gtsam/geometry/Pose2.h>
#include <cstdint>
#include <vector>
#include <optional>
#include "wfcore/video/video_types.h"

namespace wf {

    struct SE2PoseSLAMEstimate {
        gtsam::Pose2 pose;
        double residual;
        uint64_t timestamp;

        SE2PoseSLAMEstimate(
            gtsam::Pose2 pose_, double residual, uint64_t timestamp
        ) : pose(std::move(pose_)), residual(residual), timestamp(timestamp) {}
    };

}



