// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <gtsam/geometry/Pose3.h>
#include <gtsam/geometry/Pose2.h>
#include <gtsam/geometry/Point3.h>
#include <gtsam/geometry/Point2.h>
#include <opencv2/core/types.hpp>
#include <opencv2/core/eigen.hpp>
#include <opencv2/core.hpp>
#include <Eigen/Core>

namespace wf {

    // Converts OpenCV 3x3 Mat of type CV_64F to an Eigen 3d matrix
    inline Eigen::Matrix3d cvMat3ToEigen_64F(const cv::Mat& CV_M_) {
        CV_Assert(CV_M_.rows == 3 && CV_M_.cols == 3 && CV_M_.type() == CV_64F);

        Eigen::Matrix3d M_;
        cv::cv2eigen(CV_M_, M_);

        // Create GTSAM Rot3 from Eigen matrix
        return M_;
    }

    inline Eigen::Vector3d cvVec3ToEigen_64F(const cv::Mat& CV_v_) {
        CV_Assert(CV_v_.rows == 1 && CV_v_.cols == 3 && CV_v_.type() == CV_64F);

        Eigen::Vector3d v_;
        cv::cv2eigen(CV_v_, v_);

        return v_;
    }

}