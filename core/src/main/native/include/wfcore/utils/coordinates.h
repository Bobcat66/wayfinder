/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of Wayfinder.
 *
 * Copyright (C) 2025 Jesse Kane
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#pragma once

#include "wfcore/utils/cv2gtsam_interface.h"

#include <Eigen/Core>
#include <gtsam/geometry/Pose3.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/core/eigen.hpp>
#include <cassert>

// Transforms OpenCV basis vectors to WPILib basis vectors
static const Eigen::Matrix3d T_cw = (Eigen::Matrix<double,3,3,Eigen::RowMajor>() << 
      0.0,  0.0,  1.0,
     -1.0,  0.0,  0.0,
      0.0, -1.0,  0.0
).finished();

// Transforms WPILib basis vectors to OpenCV basis vectors
static const Eigen::Matrix3d T_cw_transpose = T_cw.transpose();

namespace wf {

    // Changes the frame of an OpenCV matrix to WPILib's coordinate system,
    inline Eigen::Matrix3d cvToWPILibCoords(const Eigen::Matrix3d& M_c) {
        Eigen::Matrix3d M_w = T_cw * M_c * T_cw_transpose; // Apply the basis transformation to the rotation matrix
        return M_w;
    }

    // Changes the frame of an OpenCV vector to WPILib's coordinate system,
    inline Eigen::Vector3d cvToWPILibCoords(const Eigen::Vector3d& v_c) {
        Eigen::Vector3d v_w = T_cw * v_c; // Apply the basis transformation to the translation vector
        return v_w;
    }

    // Changes the frame of an OpenCV matrix to WPILib's coordinate system,
    inline Eigen::Matrix3d WPILibToCvCoords(const Eigen::Matrix3d& M_w) {
        Eigen::Matrix3d M_c = T_cw_transpose * M_w * T_cw; // Apply the basis transformation to the rotation matrix
        return M_c;
    }

    // Changes the frame of an OpenCV vector to WPILib's coordinate system,
    inline Eigen::Vector3d WPILibToCvCoords(const Eigen::Vector3d& v_w) {
        Eigen::Vector3d v_c = T_cw_transpose * v_w; // Apply the basis transformation to the translation vector
        return v_c;
    }

    // Converts OpenCV rotation vector and translation vector to GTSAM Pose3, also changes the coordinate system from OpenCV to WPIlib
    inline gtsam::Pose3 cvPoseVecsToWPILibPose3(const cv::Mat& CV_r_c, const cv::Mat& CV_t_c){
        // Convert OpenCV rotation vector to rotation matrix
        cv::Mat CV_R_c;
        cv::Rodrigues(CV_r_c, CV_R_c);

        Eigen::Matrix3d R_c = cvMat3ToEigen_64F(CV_R_c); // Convert OpenCV rotation matrix to Eigen matrix
        Eigen::Vector3d t_c = cvVec3ToEigen_64F(CV_t_c); // Convert OpenCV translation vector to Eigen vector

        Eigen::Matrix3d R_w = cvToWPILibCoords(R_c); // Convert the rotation matrix to WPILib coordinates
        Eigen::Vector3d t_w = cvToWPILibCoords(t_c); // Convert the translation vector to WPILib coordinates

        return gtsam::Pose3(
            gtsam::Rot3(R_w),
            gtsam::Point3(t_w)
        ); 
    }

    // Converts GTSAM Pose3 in WPILib coordinates to OpenCV rotation vector and translation vector, also changes the coordinate system from WPILib to OpenCV
    inline void WPILibPose3ToCvPoseVecs(const gtsam::Pose3& pose_w, cv::Mat& CV_r_c, cv::Mat& CV_t_c) {
        // Convert the pose to Eigen
        Eigen::Matrix3d R_w = pose_w.rotation().matrix();
        Eigen::Vector3d t_w = pose_w.translation().matrix();

        // Convert the rotation matrix and translation vector to OpenCV coordinates
        Eigen::Matrix3d R_c = WPILibToCvCoords(R_w);
        Eigen::Vector3d t_c = WPILibToCvCoords(t_w);

        // Convert the rotation matrix to a rotation vector
        cv::Mat CV_R_c;
        cv::eigen2cv(R_c,CV_R_c);
        cv::Rodrigues(EigenMatrix3dToCV(R_c), CV_r_c);
        
        // Convert the translation vector to OpenCV format
        cv::eigen2cv(t_c,CV_t_c);
    }

    inline gtsam::Pose3 WPILibPose3ToCvPose3(const gtsam::Pose3& pose_w) {
        // Convert the pose to Eigen
        Eigen::Matrix3d R_w = pose_w.rotation().matrix();
        Eigen::Vector3d t_w = pose_w.translation().matrix();

        // Convert the rotation matrix and translation vector to OpenCV coordinates
        Eigen::Matrix3d R_c = WPILibToCvCoords(R_w);
        Eigen::Vector3d t_c = WPILibToCvCoords(t_w);

        return gtsam::Pose3(
            gtsam::Rot3(R_c),
            gtsam::Point3(t_c)
        ); // TODO: See if this can be made more eefficient with move semantics or something, IDK
    }
}