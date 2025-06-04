// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <opencv2/core/mat.hpp>
#include <gtsam/geometry/Pose3.h>
#include <gtsam/geometry/Pose2.h>
#include <stdint.h>
#include <Eigen/Core>
#include <vector>
#include <optional>

namespace wf {

typedef struct {
    int id;
    std::vector<Eigen::Vector2d> corners;
    double decisionMargin;
    double hammingDistance;
} AprilTagObservation;

typedef struct {
    int id;
    std::vector<Eigen::Vector2d> corners;
    double decisionMargin;
    double hammingDistance;
    gtsam::Pose3 tagPose0;
    double error0;
    gtsam::Pose3 tagPose1;
    double error1;
} RelativeTagPoseObservation;
// Observation of a single AprilTag's pose relative to the camera

typedef struct {
    std::vector<int> tagsUsed;
    gtsam::Pose3 fieldPose0;
    double error0;
    std::optional<gtsam::Pose3> fieldPose1;
    std::optional<double> error1;
} AprilTagPoseObservation;

typedef struct {
    gtsam::Pose2 pose;
    double residual;
    uint64_t timestamp;
} PoseSLAMEstimate;

typedef struct {
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;
} CameraIntrinsics;

}



