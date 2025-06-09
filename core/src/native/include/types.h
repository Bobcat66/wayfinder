// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <gtsam/geometry/Pose3.h>
#include <gtsam/geometry/Pose2.h>
#include <stdint.h>
#include <vector>
#include <optional>
#include <map>

namespace wf {

    struct AprilTagObservation {
        int id;
        std::vector<cv::Point2d> corners;
        double decisionMargin;
        double hammingDistance;
    };

    struct TagRelativePoseObservation {
        int id;
        std::vector<cv::Point2d> corners;
        double decisionMargin;
        double hammingDistance;
        cv::Mat rvec0;
        cv::Mat tvec0;
        double error0;
        cv::Mat rvec1;
        cv::Mat tvec1;
        double error1;
        TagRelativePoseObservation(
            int _id,
            const std::vector<cv::Point2d>& _corners,
            double _decisionMargin, double _hammingDistance,
            const cv::Mat& _rvec0, const cv::Mat& _tvec0, double _error0,
            const cv::Mat& _rvec1, const cv::Mat& _tvec1, double _error1
        ) : id(_id), corners(_corners), decisionMargin(_decisionMargin), hammingDistance(_hammingDistance),
        rvec0(_rvec0), tvec0(_tvec0), error0(_error0), rvec1(_rvec1), tvec1(_tvec1), error1(_error1) {}
    };
    // Observation of a single AprilTag's pose relative to the camera

    struct AprilTagPoseObservation {
        std::vector<int> tagsUsed;
        gtsam::Pose3 fieldPose0;
        double error0;
        std::optional<gtsam::Pose3> fieldPose1;
        std::optional<double> error1;
        AprilTagPoseObservation(
            const std::vector<int>& tags, 
            const gtsam::Pose3& pose0, double err0, 
            const std::optional<gtsam::Pose3>& pose1 = std::nullopt, 
            const std::optional<double>& err1 = std::nullopt
        ) : tagsUsed(tags), fieldPose0(pose0), error0(err0), fieldPose1(pose1), error1(err1) {}
    };

    struct PoseSLAMEstimate {
        gtsam::Pose2 pose;
        double residual;
        uint64_t timestamp;
    };

    struct CameraIntrinsics {
        cv::Mat cameraMatrix;
        cv::Mat distCoeffs;
    };

    struct AprilTag {
        int id;
        gtsam::Pose3 pose;
    };

}



