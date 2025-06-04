// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <gtsam/geometry/Pose3.h>
#include <gtsam/geometry/Pose2.h>
#include <gtsam/geometry/Point3.h>
#include <gtsam/geometry/Point2.h>
#include <opencv2/core/types.hpp>

namespace wf {

    // Utility functions for converting between OpenCV and GTSAM types
    gtsam::Pose3 cvPoseToGtsam(const cv::Mat& rvec, const cv::Mat& tvec);

}