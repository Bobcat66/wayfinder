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

#include "wfcore/hardware/CameraConfiguration.h"

#include "wfcore/fiducial/ApriltagConfiguration.h"
#include "wfcore/fiducial/ApriltagField.h"
#include "wfcore/fiducial/ApriltagDetection.h"
#include "wfcore/pipeline/pnpresults.h"

#include <opencv2/core.hpp>
#include <gtsam/geometry/Pose3.h>

#include <optional>
#include <unordered_set>

namespace wf {

    std::optional<ApriltagFieldPoseObservation> solvePNPApriltag(
        const std::vector<ApriltagDetection>& observations,
        const ApriltagConfiguration& tagConfig,
        const ApriltagField& tagField,
        const CameraIntrinsics& cameraIntrinsics,
        const std::unordered_set<int>& ignoreList
    ) noexcept;

    std::optional<ApriltagRelativePoseObservation> solvePNPApriltagRelative(
        const ApriltagDetection& observation,
        const ApriltagConfiguration& tagConfig,
        const CameraIntrinsics& cameraIntrinsics
    ) noexcept;

}