// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#include "utils/cv2gtsam_interface.h"

#include <opencv2/calib3d.hpp>


static const cv::Mat cvToGtsamRotTransform = (cv::Mat_<double>(3,3) << 
      0,  0,  1,
     -1,  0,  0,
      0, -1,  0
);

gtsam::Pose3 wf::cvPoseToGtsam(const cv::Mat& rvec, const cv::Mat& tvec) {
    // Convert OpenCV rotation vector to rotation matrix
    cv::Mat R;
    cv::Rodrigues(rvec, R);

    return gtsam::Pose3(
        gtsam::Rot3(R * cvToGtsamRotTransform), // Apply the transformation to the rotation matrix
        gtsam::Point3(tvec.at<double>(0), tvec.at<double>(1), tvec.at<double>(2))
    ); // placeholder
}