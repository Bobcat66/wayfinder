// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "types.h"
#include "apriltag/apriltag_field.h"

#include <optional>

namespace wf {

    std::optional<AprilTagPoseObservation> solvePnP(
        const std::vector<AprilTagDetection>& observations,
        const FieldLayout& fieldLayout,
        const CameraIntrinsics& cameraIntrinsics,
        const std::vector<int>& ignoreList
    );

    std::optional<TagRelativePoseObservation> solvePnP_TagRelative(
        const AprilTagObservation& observation,
        const FieldLayout& fieldLayout,
        const CameraIntrinsics& cameraIntrinsics
    );

}