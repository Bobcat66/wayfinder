/*
 * Copyright (c) 2025 Jesse Kane
 * You may use, distribute and modify this code under the terms of the BSD 3-Clause License.
 * For more information, see the LICENSE file in the root directory of this project.
 */

#pragma once

#include <opencv2/core/mat.hpp>
#include <gtsam/geometry/Pose3.h>
#include <gtsam/geometry/Pose2.h>
#include <stdint.h>
#include <Eigen/Core>

namespace svcore {

typedef struct {
    int id;
    std::vector<Eigen::Vector2d> corners;
    double decisionMargin;
    double hammingDistance;
} FiducialObservation;

typedef struct {
    int id;
    std::vector<Eigen::Vector2d> corners;
    double decisionMargin;
    double hammingDistance;
    gtsam::Pose3 tagpose0;
    double error0;
    gtsam::Pose3 tagpose1;
    double error1;
} FiducialPoseObservation;

typedef struct {
    gtsam::Pose2 pose;
    double residual;
    uint64_t timestamp;
} PoseSLAMEstimate;

}



