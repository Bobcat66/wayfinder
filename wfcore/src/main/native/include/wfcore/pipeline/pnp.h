// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "wfcore/types.h"
#include "wfcore/configuration/configurations.h"
#include "wfcore/fiducial/ApriltagField.h"
#include "wfcore/fiducial/ApriltagDetection.h"

#include <optional>

namespace wf {

    std::optional<ApriltagFieldPoseObservation> solvePNPApriltag(
        const std::vector<ApriltagDetection>& observations,
        const ApriltagField& fieldLayout,
        const CameraIntrinsics& cameraIntrinsics,
        const std::vector<int>& ignoreList
    );

    std::optional<ApriltagRelativePoseObservation> solvePNPApriltagRelative(
        const ApriltagDetection& observation,
        const ApriltagField& fieldLayout,
        const CameraIntrinsics& cameraIntrinsics
    );

}