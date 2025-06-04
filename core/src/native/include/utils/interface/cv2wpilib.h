// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <Eigen/Core>
#include <gtsam/geometry/Pose3.h>
#include <opencv2/core/mat.hpp>


static const Eigen::Matrix3d T_cw = (Eigen::Matrix<double,3,3,Eigen::RowMajor>() << 
      0.0,  0.0,  1.0,
     -1.0,  0.0,  0.0,
      0.0, -1.0,  0.0
).finished(); // Transforms OpenCV coordinates to WPILib coordinates

static const Eigen::Matrix3d T_wc = T_cw.inverse(); // Transforms WPILib coordinates to OpenCV coordinates

namespace wf {
    // Converts OpenCV rotation vector and translation vector to GTSAM Pose3, also changes the coordinate system from OpenCV to WPIlib
    gtsam::Pose3 cvPoseToWPILibPose(const cv::Mat& CV_r_c, const cv::Mat& CV_t_c);

    // Changes the frame of an OpenCV matrix to WPILib's coordinate system,
    inline Eigen::Matrix3d cvToWPILibCoords(const Eigen::Matrix3d& M_c) {
        Eigen::Matrix3d M_w = T_wc * M_c * T_cw; // Apply the basis transformation to the rotation matrix
        return M_w;
    }

    // Changes the frame of an OpenCV vector to WPILib's coordinate system,
    inline Eigen::Vector3d cvToWPILibCoords(const Eigen::Vector3d& t_c) {
        Eigen::Vector3d t_w = t_c * T_cw; // Apply the basis transformation to the translation vector
        return t_w;
    }
}