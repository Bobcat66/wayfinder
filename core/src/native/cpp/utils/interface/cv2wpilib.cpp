// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#include "utils/geometry.h"
#include "utils/interface/cv2wpilib.h"
#include "utils/interface/cv2gtsam.h"

#include <opencv2/calib3d.hpp>

using namespace wf;

gtsam::Pose3 cvPoseToWPILibPose(const cv::Mat& CV_r_c, const cv::Mat& CV_t_c) {

    // Convert OpenCV rotation vector to rotation matrix
    cv::Mat CV_R_c;
    cv::Rodrigues(CV_r_c, CV_R_c);

    Eigen::Matrix3d R_c = cvMat3ToEigen_64F(CV_R_c); // Convert OpenCV rotation matrix to Eigen matrix
    Eigen::Vector3d t_c = cvVec3ToEigen_64F(CV_t_c); // Convert OpenCV translation vector to Eigen vector

    Eigen::Matrix3d R_w = cvToWPILibCoords(R_c); // Convert the rotation matrix to WPILib coordinates
    Eigen::Vector3d t_w = cvToWPILibCoords(t_c); // Convert the translation vector to WPILib coordinates

    return gtsam::Pose3(
        gtsam::Rot3(R_w), // Apply the transformation to the rotation matrix
        gtsam::Point3(t_w)
    ); // placeholder
}