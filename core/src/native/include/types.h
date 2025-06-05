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

    struct AprilTagPoseObservation {
        int id;
        std::vector<cv::Point2d> corners;
        double decisionMargin;
        double hammingDistance;
        gtsam::Pose3 tagPose0;
        double error0;
        gtsam::Pose3 tagPose1;
        double error1;
    };
    // Observation of a single AprilTag's pose relative to the camera

    struct CameraPoseObservation {
        std::vector<int> tagsUsed;
        gtsam::Pose3 fieldPose0;
        double error0;
        std::optional<gtsam::Pose3> fieldPose1;
        std::optional<double> error1;
        CameraPoseObservation(
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

    struct FieldLayout {
        std::map<int, AprilTag> aprilTags;
        std::string tagFamily;
        double tagSize;
        const AprilTag* getTag(int id) const {
            auto it = aprilTags.find(id);
            if (it != aprilTags.end()) {
                return &it->second;
            }
            return nullptr; // Not found
        }
    };

}



