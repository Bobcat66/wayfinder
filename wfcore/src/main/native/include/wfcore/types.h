// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <gtsam/geometry/Pose3.h>
#include <gtsam/geometry/Pose2.h>
#include <cstdint>
#include <vector>
#include <optional>

namespace wf {

    struct ApriltagRelativePoseObservation {
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
        ApriltagRelativePoseObservation(
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
    // Observation of a single Apriltag's pose relative to the camera

    struct ApriltagFieldPoseObservation {
        std::vector<int> tagsUsed;
        gtsam::Pose3 fieldPose0;
        double error0;
        std::optional<gtsam::Pose3> fieldPose1;
        std::optional<double> error1;

        ApriltagFieldPoseObservation(
            std::vector<int> tagsUsed_, 
            gtsam::Pose3 fieldPose0_, double error0_
        ) : tagsUsed(std::move(tagsUsed_)), 
            fieldPose0(std::move(fieldPose0_)), error0(error0_), 
            fieldPose1(std::nullopt), error1(std::nullopt) {}
        ApriltagFieldPoseObservation(
            std::vector<int> tagsUsed_, 
            gtsam::Pose3 fieldPose0_, double error0_,
            gtsam::Pose3 fieldPose1_, double error1_
        ) : tagsUsed(std::move(tagsUsed_)), 
            fieldPose0(std::move(fieldPose0_)), error0(error0_), 
            fieldPose1(std::make_optional(std::move(fieldPose1_))), error1(std::make_optional(error1_)) {}
    };

    struct ObjectDetection {
        int objectClass;
        double confidence;
        double percentArea;
        std::vector<cv::Point2d> cornerPixels;
        std::vector<cv::Point2d> cornerAngles;
        ObjectDetection(
            int objectClass_, double confidence_, double percentArea_,
            std::vector<cv::Point2d> cornerPixels_,
            std::vector<cv::Point2d> cornerAngles_
        ) : objectClass(objectClass_), confidence(confidence_), percentArea(percentArea_),
            cornerPixels(std::move(cornerPixels_)), cornerAngles(std::move(cornerAngles_)) {}
    };

    struct PipelineResult {
        std::optional<std::vector<ApriltagRelativePoseObservation>> aprilTagPoses;
        std::optional<ApriltagFieldPoseObservation> cameraPose;
        cv::Mat im;
        uint64_t timestampMicros;
    };

    struct SE2PoseSLAMEstimate {
        gtsam::Pose2 pose;
        double residual;
        uint64_t timestamp;

        SE2PoseSLAMEstimate(
            gtsam::Pose2 pose_, double residual, uint64_t timestamp
        ) : pose(std::move(pose_)), residual(residual), timestamp(timestamp) {}
    };

}



