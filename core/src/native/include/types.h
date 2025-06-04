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

    struct AprilTagObservation {
        int id;
        std::vector<cv::Point2d> corners;
        double decisionMargin;
        double hammingDistance;
    };

    struct RelativeTagPoseObservation {
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

    struct AprilTagPoseObservation {
        std::vector<int> tagsUsed;
        gtsam::Pose3 fieldPose0;
        double error0;
        std::optional<gtsam::Pose3> fieldPose1;
        std::optional<double> error1;
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



