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

namespace wf {

    struct AprilTagPoseObservation {
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
        AprilTagPoseObservation(
            int id_,
            const std::vector<cv::Point2d> corners_,
            double decisionMargin_, double hammingDistance_,
            cv::Mat rvec0_, cv::Mat tvec0_, double error0_,
            cv::Mat rvec1_, cv::Mat tvec1_, double error1_
        ) : id(id_), 
            corners(std::move(corners_)), 
            decisionMargin(decisionMargin_), hammingDistance(hammingDistance_),
            rvec0(std::move(rvec0_)), tvec0(std::move(tvec0_)), error0(error0_), 
            rvec1(std::move(rvec1_)), tvec1(std::move(tvec1_)), error1(error1_) {}
    };
    // Observation of a single AprilTag's pose relative to the camera

    struct CameraPoseObservation {
        std::vector<int> tagsUsed;
        gtsam::Pose3 fieldPose0;
        double error0;
        std::optional<gtsam::Pose3> fieldPose1;
        std::optional<double> error1;

        CameraPoseObservation(
            std::vector<int> tagsUsed_, 
            gtsam::Pose3 fieldPose0_, double error0_
        ) : tagsUsed(std::move(tagsUsed_)), 
            fieldPose0(std::move(fieldPose0_)), error0(error0_), 
            fieldPose1(std::nullopt), error1(std::nullopt) {}
        CameraPoseObservation(
            std::vector<int> tagsUsed_, 
            gtsam::Pose3 fieldPose0_, double error0_,
            gtsam::Pose3 fieldPose1_, double error1_
        ) : tagsUsed(std::move(tagsUsed_)), 
            fieldPose0(std::move(fieldPose0_)), error0(error0_), 
            fieldPose1(std::make_optional(std::move(fieldPose1_))), error1(std::make_optional(error1_)) {}
    };

    struct SE2PoseSLAMEstimate {
        gtsam::Pose2 pose;
        double residual;
        uint64_t timestamp;

        SE2PoseSLAMEstimate(
            gtsam::Pose2 pose_, double residual, uint64_t timestamp
        ) : pose(std::move(pose_)), residual(residual), timestamp(timestamp) {}
    };

    struct CameraIntrinsics {
        cv::Mat cameraMatrix;
        cv::Mat distCoeffs;
    };

}



