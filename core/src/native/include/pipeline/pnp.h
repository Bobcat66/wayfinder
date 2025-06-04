// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "types.h"
#include "field.h"
#include "utils/cv2gtsam_interface.h"
#include "utils/gtsam_geometry.h"

namespace wf {

    RelativeTagPoseObservation solvePnP_TagRelative(
        const AprilTagObservation& observations,
        const FieldLayout& fieldLayout,
        const CameraIntrinsics& cameraIntrinsics
    );

    AprilTagPoseObservation solvePnP(
        const std::vector<AprilTagObservation>& observations,
        const FieldLayout& fieldLayout,
        const CameraIntrinsics& cameraIntrinsics,
        const std::vector<int>& ignoreList
    );

}