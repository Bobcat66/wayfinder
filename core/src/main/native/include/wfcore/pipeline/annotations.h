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

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <gtsam/geometry/Pose3.h>
#include "wfcore/hardware/CameraConfiguration.h"
#include <vector>
#include "wfcore/inference/InferenceEngine.h"
#include "wfcore/fiducial/ApriltagDetection.h"
#include "wfcore/pipeline/pnp.h"

namespace wf {
    int drawTag3D(
        cv::Mat& image,
        const ApriltagRelativePoseObservation& observation,
        const CameraIntrinsics& intrinsics,
        double tagSize
    );
    int drawBbox(cv::Mat& image, const ObjectDetection& detection);
    int drawTag(cv::Mat& image, const ApriltagDetection& detection);
    int drawCamLabel(cv::Mat& image, const std::string& camera_label);
}

