// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtsam/geometry/Pose3.h>
#include <gtsam/geometry/Pose2.h>
#include <gtsam/geometry/Point3.h>
#include <gtsam/geometry/Point2.h>
#include <opencv2/core.hpp>
#include <opencv2/core/eigen.hpp>
#include <Eigen/Core>

// For some type safety, and convenience. Maybe remove?
namespace wf {

    // Converts OpenCV 3x3 Mat of type CV_64F to an Eigen 3d matrix
    inline Eigen::Matrix3d cvMat3ToEigen_64F(const cv::Mat& CV_M) {
        assert(CV_M.rows == 3 && CV_M.cols == 3 && CV_M.type() == CV_64F);

        Eigen::Matrix3d Eigen_M;
        cv::cv2eigen(CV_M, Eigen_M);

        // Create GTSAM Rot3 from Eigen matrix
        return Eigen_M;
    }

    inline Eigen::Vector3d cvVec3ToEigen_64F(const cv::Mat& CV_v) {
        assert(CV_v.rows == 1 && CV_v.cols == 3 && CV_v.type() == CV_64F);

        Eigen::Vector3d Eigen_v;
        cv::cv2eigen(CV_v, Eigen_v);

        return Eigen_v;
    }

    inline cv::Mat EigenVector3dToCV(const Eigen::Vector3d Eigen_v) {
        cv::Mat CV_v;

        cv::eigen2cv(Eigen_v,CV_v);
        
        return CV_v;
    }

    inline cv::Mat EigenMatrix3dToCV(const Eigen::Matrix3d Eigen_M) {
        cv::Mat CV_M;

        cv::eigen2cv(Eigen_M,CV_M);
        
        return CV_M;
    }

}