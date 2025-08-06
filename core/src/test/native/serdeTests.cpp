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

#include "wfcore/common/serde/legacy.h"
#include "wfcore/common/logging/LoggerManager.h"
#include "wfcore/pipeline/pnp.h"
#include "wips/pose3.wips.h"
#include "wips/apriltag_field_pose_observation.wips.h"
#include "wips/apriltag_relative_pose_observation.wips.h"
#include "wips/wips_runtime.h"

#include <chrono>
#include <gtest/gtest.h>
#include <vector>

static wf::loggerPtr logger = wf::LoggerManager::getInstance().getLogger("serdeTests",wf::LogGroup::General);

TEST(serdeTests, Pose3SerdeTest) {
    gtsam::Pose3 pose(gtsam::Rot3::RzRyRx(0.1, 0.2, 0.3), gtsam::Point3(1.0, 2.0, 3.0));
    auto wipsbin = wf::packPose3(pose);
    wipsbin->offset = 0; // Reset offset to read the whole data
    auto deserializedPose = wf::unpackPose3(wipsbin);
    // Check if the original pose and deserialized pose are approximately equal
    EXPECT_TRUE(pose.equals(deserializedPose, 1e-9)) << "Deserialized pose does not match original pose";
}

TEST(serdeTests, FPOSerdeTest) {
    wf::ApriltagFieldPoseObservation poseObservation(
        {1,4,7},
        gtsam::Pose3(gtsam::Rot3::RzRyRx(0.1, 0.2, 0.3), gtsam::Point3(1.0, 2.0, 3.0)),
        0.5,
        std::nullopt,
        std::nullopt
    );
    auto wipsbin = wf::packApriltagFieldPoseObservation(poseObservation);
    wipsbin->offset = 0; // Reset offset to read the whole data
    auto deserializedPoseObservation = wf::unpackApriltagFieldPoseObservation(wipsbin);
    // Check if the original pose and deserialized pose are approximately equal
    EXPECT_EQ(poseObservation.tagsUsed, deserializedPoseObservation.tagsUsed) << "Tags used do not match";
    EXPECT_TRUE(poseObservation.fieldPose0.equals(deserializedPoseObservation.fieldPose0, 1e-9)) 
        << "Deserialized field pose 0 does not match original field pose";
    EXPECT_DOUBLE_EQ(poseObservation.error0, deserializedPoseObservation.error0) 
        << "Deserialized error 0 does not match original error";
    EXPECT_FALSE(deserializedPoseObservation.fieldPose1.has_value()) 
        << "Deserialized field pose 1 should not have a value";
    EXPECT_FALSE(deserializedPoseObservation.error1.has_value()) 
        << "Deserialized error 1 should not have a value";
}

TEST(serdeTests, pose3WIPSCharacterizationTest) {
    using clock = std::chrono::steady_clock;
    std::chrono::time_point<clock> start, end;
    for (int i = 0; i < 5; ++i) {
        gtsam::Pose3 pose(gtsam::Rot3::RzRyRx(0.1 * i, 0.2 * i, 0.3 * i), gtsam::Point3(1.0 * i, 2.0 * i, 3.0 * i));
        start = clock::now();
        auto wipsbin = wf::packPose3(pose);
        end = clock::now();
        logger->info(
            "Pose3 Serialization + Shims took {} ns for iteration {}", 
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count(),
            i
        );
        wipsbin->offset = 0; // Reset offset to read the whole data
        // Deserialize the pose3
        start = clock::now();
        auto deserializedPose = wf::unpackPose3(wipsbin);
        end = clock::now();
        logger->info(
            "Pose3 Deserialization + Shims took {} ns for iteration {}", 
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count(),
            i
        );
        wipsbin->offset = 0;
        auto wipsstruct = wips_pose3_create();
        start = clock::now();
        wips_decode_pose3(wipsstruct, wipsbin);
        end = clock::now();
        logger->info(
            "WIPS Pose3 Deserialization took {} ns for iteration {}", 
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count(),
            i
        );
        wipsbin->offset = 0; // Reset offset to read the whole data
        start = clock::now();
        wips_encode_pose3(wipsbin, wipsstruct);
        end = clock::now();
        logger->info(
            "WIPS Pose3 Serialization took {} ns for iteration {}", 
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count(),
            i
        );
        wips_pose3_free_resources(wipsstruct);
        wips_blob_destroy(wipsbin);
    }
}

TEST(serdeTests, FPOSerdeCharacterizationTest) {
    using clock = std::chrono::steady_clock;
    std::chrono::time_point<clock> start, end;
    for (int i = 0; i < 5; ++i) {
        wf::ApriltagFieldPoseObservation poseObservation(
            {1,4,7},
            gtsam::Pose3(gtsam::Rot3::RzRyRx(0.1 * i, 0.2 * i, 0.3 * i), gtsam::Point3(1.0 * i, 2.0 * i, 3.0 * i)),
            0.5 * i,
            std::nullopt,
            std::nullopt
        );
        start = clock::now();
        auto wipsbin = wf::packApriltagFieldPoseObservation(poseObservation);
        end = clock::now();
        logger->info(
            "FPOSerialization + Shims took {} ns for iteration {}", 
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count(),
            i
        );
        wipsbin->offset = 0; // Reset offset to read the whole data
        // Deserialize the poseObservation
        start = clock::now();
        auto deserializedPoseObservation = wf::unpackApriltagFieldPoseObservation(wipsbin);
        end = clock::now();
        logger->info(
            "FPODeserialization + Shims took {} ns for iteration {}", 
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count(),
            i
        );
        wipsbin->offset = 0;
        auto wipsstruct = wips_apriltag_field_pose_observation_create();
        start = clock::now();
        wips_decode_apriltag_field_pose_observation(wipsstruct, wipsbin);
        end = clock::now();
        logger->info(
            "WIPS FPO Deserialization took {} ns for iteration {}", 
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count(),
            i
        );
        wipsbin->offset = 0; // Reset offset to read the whole data
        start = clock::now();
        wips_encode_apriltag_field_pose_observation(wipsbin, wipsstruct);
        end = clock::now();
        logger->info(
            "WIPS FPO Serialization took {} ns for iteration {}", 
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count(),
            i
        );
        wips_apriltag_field_pose_observation_free_resources(wipsstruct);
        wips_blob_destroy(wipsbin);
    }
}