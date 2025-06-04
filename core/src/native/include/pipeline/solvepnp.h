// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "wftypes.h"
#include "field.h"

namespace wf {

RelativeTagPoseObservation solvePnP_SingleTagRelative(
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