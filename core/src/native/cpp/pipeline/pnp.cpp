// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#include "pipeline/pnp.h"
#include "utils/cv2gtsam_interface.h"
#include "utils/coordinates.h"
#include "utils/geometry.h"

#include <opencv2/calib3d.hpp>
#include <gtsam/geometry/Pose3.h>
#include <array>

using namespace wf;

std::optional<TagRelativePoseObservation> solvePnP_TagRelative(
    const AprilTagObservation& observation,
    const FieldLayout& fieldLayout,
    const CameraIntrinsics& cameraIntrinsics
) {
    std::vector<cv::Mat> rvecs, tvecs;
    std::vector<double> reprojectionErrors;
    std::vector<cv::Point2d> imagePoints;
    std::vector<cv::Point3d> objectPoints;
    for (const auto& corner : observation.corners) {
        imagePoints.push_back(corner);
    }
    objectPoints.emplace_back(
        -fieldLayout.tagSize / 2.0,
        fieldLayout.tagSize / 2.0, 
        0.0
    );
    objectPoints.emplace_back(
        fieldLayout.tagSize / 2.0, 
        fieldLayout.tagSize / 2.0,
        0.0
    );
    objectPoints.emplace_back(
        fieldLayout.tagSize / 2.0, 
        -fieldLayout.tagSize / 2.0,
        0.0
    );
    objectPoints.emplace_back(
        -fieldLayout.tagSize / 2.0, 
        -fieldLayout.tagSize / 2.0,
        0.0
    );
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
        return std::nullopt; //PnP was not successful, give up
    } else {
        return std::optional<TagRelativePoseObservation>{
            std::in_place,
            observation.id,
            observation.corners,
            observation.decisionMargin,
            observation.hammingDistance,
            rvecs[0], tvecs[0], reprojectionErrors[0],
            rvecs[1], tvecs[1], reprojectionErrors[1]
        };
    }
}

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
        gtsam::Pose3 tagPose_c = WPILibPose3ToCvPose3(tag->pose);
        // Emplacing object points based on the tag's pose
        // TODO: Check the offsets
        // These are in OpenCV coordinates
        cornerPoses[0] = translatePoseFrame(
            tagPose_c, 
            -fieldLayout.tagSize / 2.0,
            -fieldLayout.tagSize / 2.0, 
            0.0 
        );
        cornerPoses[1] = translatePoseFrame(
            tagPose_c, 
            fieldLayout.tagSize / 2.0, 
            -fieldLayout.tagSize / 2.0,
            0.0
        );
        cornerPoses[2] = translatePoseFrame(
            tagPose_c, 
            fieldLayout.tagSize / 2.0, 
            fieldLayout.tagSize / 2.0,
            0.0
        );
        cornerPoses[3] = translatePoseFrame(
            tagPose_c,
            -fieldLayout.tagSize / 2.0, 
            fieldLayout.tagSize / 2.0,
            0.0
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
        objectPoints.clear();
        objectPoints.emplace_back(
            -fieldLayout.tagSize / 2.0,
            fieldLayout.tagSize / 2.0, 
            0.0
        );
        objectPoints.emplace_back(
            fieldLayout.tagSize / 2.0, 
            fieldLayout.tagSize / 2.0,
            0.0
        );
        objectPoints.emplace_back(
            fieldLayout.tagSize / 2.0, 
            -fieldLayout.tagSize / 2.0,
            0.0
        );
        objectPoints.emplace_back(
            -fieldLayout.tagSize / 2.0, 
            -fieldLayout.tagSize / 2.0,
            0.0
        );
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
            return std::nullopt; // Failed to solve PnP, give up
        } else {
            const gtsam::Pose3 fieldToTagPose = tagPoses[0];
            const gtsam::Pose3 tagPose0_w = cvPoseVecsToWPILibPose3(rvecs[0], tvecs[0]);
            const gtsam::Pose3 tagPose1_w = cvPoseVecsToWPILibPose3(rvecs[1], tvecs[1]);
            const gtsam::Pose3 fieldPose0_w = fieldToTagPose.compose(tagPose0_w.inverse());
            const gtsam::Pose3 fieldPose1_w = fieldToTagPose.compose(tagPose0_w.inverse());
            double error0 = reprojectionErrors[0];
            double error1 = reprojectionErrors[1];
            return std::optional<AprilTagPoseObservation>{
                std::in_place,
                tagsUsed,
                fieldPose0_w,
                error0,
                std::optional<gtsam::Pose3>(fieldPose1_w),
                std::optional<double>(error1)
            }; // Todo: Optimize this construction
        }
    } else {
        cv::Mat tvec, rvec;
        std::vector<double> reprojectionErrors;
        bool success = cv::solvePnPGeneric(
            objectPoints,
            imagePoints,
            cameraIntrinsics.cameraMatrix,
            cameraIntrinsics.distCoeffs,
            rvec,
            tvec,
            false,
            cv::SOLVEPNP_SQPNP,
            cv::noArray(),
            cv::noArray(),
            reprojectionErrors
        );
        if (!success) {
            return std::nullopt; // Failed to solve PnP, give up
        } else {
            return std::optional<AprilTagPoseObservation>{
                std::in_place,
                tagsUsed,
                cvPoseVecsToWPILibPose3(rvec,tvec),
                reprojectionErrors[0],
                std::nullopt,
                std::nullopt
            };
        }
        
        return std::nullopt; // Placeholder for multiple tags case
    }
};



