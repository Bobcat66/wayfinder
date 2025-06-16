// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "wfcore/fiducial/ApriltagDetector.h"
#include <unordered_set>

namespace wf {

    struct ObjectDetectPipelineConfiguration {
        std::string model;
        double confidence_threshold;
    };

    struct CameraIntrinsics {
        cv::Mat cameraMatrix;
        cv::Mat distCoeffs;
    };

    struct CameraConfiguration {
        int id;
        CameraIntrinsics intrinsics;
        
    };
}