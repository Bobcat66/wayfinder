/*
 * SPDX-License-Identifier: GPL-3.0-or-later
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


#include "wfcore/pipeline/pnp.h"

#include "wfcore/utils/cv2gtsam_interface.h"
#include "wfcore/utils/coordinates.h"
#include "wfcore/utils/geometry.h"
#include "wfcore/common/logging.h"

#include <opencv2/calib3d.hpp>
#include <gtsam/geometry/Pose3.h>
#include <array>

namespace wf {

    std::optional<ApriltagFieldPoseObservation> solvePNPApriltag(
        const std::vector<ApriltagDetection>& detections,
        const ApriltagConfiguration& tagConfig,
        const ApriltagField& tagField,
        const CameraIntrinsics& cameraIntrinsics,
        const std::unordered_set<int>& ignoreList
    ) noexcept {

        // Statically allocate buffers so they are not reallocated every frame
        static thread_local std::vector<cv::Point3d> objectPoints;
        static thread_local std::vector<cv::Point2d> imagePoints;
        static thread_local std::vector<cv::Mat> rvecs, tvecs;
        static thread_local std::vector<double> reprojectionErrors;
        static thread_local std::vector<gtsam::Pose3> tagPoses;

        objectPoints.clear();
        imagePoints.clear();
        tagPoses.clear();

        objectPoints.reserve(detections.size() * 4);
        imagePoints.reserve(detections.size() * 4);
        tagPoses.reserve(detections.size());

        std::vector<int> tagsUsed;
        tagsUsed.reserve(detections.size());

        // We might be reserving more space than we need, as tags could be filtered out by the ignoreList.

        // Memory pool for storing corner poses
        std::array<gtsam::Pose3, 4> cornerPoses;
        for (const auto& det : detections) {
            if (std::find(ignoreList.begin(), ignoreList.end(), det.id) != ignoreList.end()) {
                WF_DEBUGLOG(globalLogger(),"Ignoring tag {} for PnP",det.id);
                continue; // Skip ignored tags
            }
            const wf::Apriltag* tag = tagField.getTag(det.id);
            if (!tag) {
                WF_DEBUGLOG(globalLogger(),"Tag {} not found in field",det.id);
                continue; // Skip tags without a pose
            }
            tagsUsed.push_back(det.id);
            tagPoses.push_back(tag->pose);

            // Assuming the tag corners are in the order: top-left, top-right, bottom-right, bottom-left
            for (const auto& corner : det.corners) {
                imagePoints.push_back(corner);
            }
            gtsam::Pose3 tagPose_c = WPILibPose3ToCvPose3(tag->pose);
            // Emplacing object points based on the tag's pose
            // TODO: Check the offsets
            // These are in OpenCV coordinates
            cornerPoses[0] = translatePoseFrame(
                tagPose_c, 
                -tagConfig.tagSize / 2.0,
                -tagConfig.tagSize / 2.0, 
                0.0 
            );
            cornerPoses[1] = translatePoseFrame(
                tagPose_c, 
                tagConfig.tagSize / 2.0, 
                -tagConfig.tagSize / 2.0,
                0.0
            );
            cornerPoses[2] = translatePoseFrame(
                tagPose_c, 
                tagConfig.tagSize / 2.0, 
                tagConfig.tagSize / 2.0,
                0.0
            );
            cornerPoses[3] = translatePoseFrame(
                tagPose_c,
                -tagConfig.tagSize / 2.0, 
                tagConfig.tagSize / 2.0,
                0.0
            );
            for (const auto& cornerPose : cornerPoses) {
                objectPoints.emplace_back(cornerPose.x(), cornerPose.y(), cornerPose.z());
            }
        }

        if (tagsUsed.size() == 0) {
            return std::nullopt; // No tags found, give up
        } else if (tagsUsed.size() == 1) {
            WF_DEBUGLOG(globalLogger(),"1 valid tag found. Using IPPE Square algorithm");
            rvecs.clear();
            tvecs.clear();
            reprojectionErrors.clear();
            objectPoints.clear();
            
            objectPoints.emplace_back(
                -tagConfig.tagSize / 2.0,
                tagConfig.tagSize / 2.0, 
                0.0
            );
            objectPoints.emplace_back(
                tagConfig.tagSize / 2.0, 
                tagConfig.tagSize / 2.0,
                0.0
            );
            objectPoints.emplace_back(
                tagConfig.tagSize / 2.0, 
                -tagConfig.tagSize / 2.0,
                0.0
            );
            objectPoints.emplace_back(
                -tagConfig.tagSize / 2.0, 
                -tagConfig.tagSize / 2.0,
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
                WF_DEBUGLOG(globalLogger(),"IPPE Square calculation failed");
                return std::nullopt; // Failed to solve PnP, give up
            } else {
                const gtsam::Pose3 fieldToTagPose = tagPoses[0];
                const gtsam::Pose3 tagPose0_w = cvPoseVecsToWPILibPose3(rvecs[0], tvecs[0]);
                const gtsam::Pose3 tagPose1_w = cvPoseVecsToWPILibPose3(rvecs[1], tvecs[1]);
                const gtsam::Pose3 fieldPose0_w = fieldToTagPose.compose(tagPose0_w.inverse());
                const gtsam::Pose3 fieldPose1_w = fieldToTagPose.compose(tagPose1_w.inverse());
                double error0 = reprojectionErrors[0];
                double error1 = reprojectionErrors[1];
                return std::optional<ApriltagFieldPoseObservation>{
                    std::in_place,
                    std::move(tagsUsed),
                    std::move(fieldPose0_w),
                    error0,
                    std::move(fieldPose1_w),
                    error1
                }; // Todo: Optimize this construction
            }
        } else {
            WF_DEBUGLOG(globalLogger(),">1 valid tags found. Using SQPnP algorithm");
            tvecs.clear();
            rvecs.clear();
            reprojectionErrors.clear();

            bool success = cv::solvePnPGeneric(
                objectPoints,
                imagePoints,
                cameraIntrinsics.cameraMatrix,
                cameraIntrinsics.distCoeffs,
                rvecs,
                tvecs,
                false,
                cv::SOLVEPNP_SQPNP,
                cv::noArray(),
                cv::noArray(),
                reprojectionErrors
            );
            if (!success) {
                WF_DEBUGLOG(globalLogger(),"SQPnP calculation failed");
                return std::nullopt; // Failed to solve PnP, give up
            } else {
                return std::optional<ApriltagFieldPoseObservation>(
                    std::in_place,
                    std::move(tagsUsed),
                    cvPoseVecsToWPILibPose3(rvecs[0],tvecs[0]),
                    reprojectionErrors[0]
                );
            }
        }
    }

    std::optional<ApriltagRelativePoseObservation> solvePNPApriltagRelative(
        const ApriltagDetection& detection,
        const ApriltagConfiguration& tagConfig,
        const CameraIntrinsics& cameraIntrinsics
    ) noexcept {
        static thread_local std::vector<cv::Mat> rvecs, tvecs;
        static thread_local std::vector<double> reprojectionErrors;
        static thread_local std::vector<cv::Point2d> imagePoints;
        static thread_local std::vector<cv::Point3d> objectPoints;

        rvecs.clear();
        tvecs.clear();
        reprojectionErrors.clear();
        imagePoints.clear();
        objectPoints.clear();
        
        for (const auto& corner : detection.corners) {
            imagePoints.push_back(corner);
        }
        objectPoints.emplace_back(
            -tagConfig.tagSize / 2.0,
            tagConfig.tagSize / 2.0, 
            0.0
        );
        objectPoints.emplace_back(
            tagConfig.tagSize / 2.0, 
            tagConfig.tagSize / 2.0,
            0.0
        );
        objectPoints.emplace_back(
            tagConfig.tagSize / 2.0, 
            -tagConfig.tagSize / 2.0,
            0.0
        );
        objectPoints.emplace_back(
            -tagConfig.tagSize / 2.0, 
            -tagConfig.tagSize / 2.0,
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
            WF_DEBUGLOG(globalLogger(),"PnP calculation failed");
            return std::nullopt; //PnP was not successful, give up
        } else {
            return std::optional<ApriltagRelativePoseObservation>{
                std::in_place,
                detection.id,
                cvPoseVecsToWPILibPose3(rvecs[0], tvecs[0]), reprojectionErrors[0],
                cvPoseVecsToWPILibPose3(rvecs[1], tvecs[1]), reprojectionErrors[1]
            };
        }
    }


}



