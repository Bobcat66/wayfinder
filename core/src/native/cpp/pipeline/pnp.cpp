// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#include "pipeline/pnp.h"
#include "utils/cv2gtsam_interface.h"
#include "utils/gtsam_geometry.h"

#include <opencv2/calib3d.hpp>
#include <gtsam/geometry/Pose3.h>
#include <array>
#include <optional>

using namespace wf;

std::optional<AprilTagPoseObservation> solvePnP(
    const std::vector<AprilTagObservation>& observations,
    const FieldLayout& fieldLayout,
    const CameraIntrinsics& cameraIntrinsics,
    const std::vector<int>& ignoreList
) {
    // TODO: Profile and check if this causes unacceptable heap churn, if so,
    // consider switching to an arena allocator or some other memory management strategy.
    // IDT it'll cause issues, but it's worth noting
    std::vector<cv::Point3d> objectPoints;
    objectPoints.reserve(observations.size() * 4);
    std::vector<cv::Point2d> imagePoints;
    imagePoints.reserve(observations.size() * 4);
    std::vector<int> tagsUsed;
    tagsUsed.reserve(observations.size());
    std::vector<gtsam::Pose3> tagPoses;
    tagPoses.reserve(observations.size());
    // We might be reserving more space than we need, as tags could be filtered out by the ignoreList.

    // Memory pool for storing corner poses
    std::array<gtsam::Pose3, 4> cornerPoses;
    for (const auto& obs : observations) {
        if (std::find(ignoreList.begin(), ignoreList.end(), obs.id) != ignoreList.end()) {
            continue; // Skip ignored tags
        }
        const wf::AprilTag* tag = fieldLayout.getTag(obs.id);
        if (!tag) {
            continue; // Skip tags without a pose
        }
        tagsUsed.push_back(obs.id);
        tagPoses.push_back(tag->pose);

        // Assuming the tag corners are in the order: top-left, top-right, bottom-right, bottom-left
        for (const auto& corner : obs.corners) {
            imagePoints.push_back(corner);
        }
        // Emplacing object points based on the tag's pose
        // TODO: Check the offsets
        cornerPoses[0] = translatePoseFrame(
            tag->pose, 
            0.0,
            fieldLayout.tagSize / 2.0, 
            fieldLayout.tagSize / 2.0 
        );
        cornerPoses[1] = translatePoseFrame(
            tag->pose, 
            0.0,
            fieldLayout.tagSize / 2.0, 
            -fieldLayout.tagSize / 2.0 
        );
        cornerPoses[2] = translatePoseFrame(
            tag->pose, 
            0.0,
            -fieldLayout.tagSize / 2.0, 
            -fieldLayout.tagSize / 2.0 
        );
        cornerPoses[3] = translatePoseFrame(
            tag->pose, 
            0.0,
            -fieldLayout.tagSize / 2.0, 
            fieldLayout.tagSize / 2.0 
        );
        for (const auto& cornerPose : cornerPoses) {
            objectPoints.emplace_back(cornerPose.x(), cornerPose.y(), cornerPose.z());
        }
    }

    if (tagsUsed.size() == 0) {
        return std::nullopt;
    } else if (tagsUsed.size() == 1) {
        std::vector<cv::Mat> rvecs, tvecs;
        std::vector<double> reprojectionErrors;
        bool success = cv::solvePnPGeneric(
            objectPoints,
            imagePoints,
            cameraIntrinsics.cameraMatrix,
            cameraIntrinsics.distCoeffs,
            rvecs,
            tvecs,
            false,
            cv::SOLVEPNP_IPPE_SQUARE,
            cv::noArray(),
            cv::noArray(),
            reprojectionErrors
        );
        if (!success) {
            return std::nullopt; // Failed to solve PnP
        } else {
            
            return std::nullopt; // Placeholder for single tag case
        }
    } else {
        return std::nullopt; // Placeholder for multiple tags case
    }
};



