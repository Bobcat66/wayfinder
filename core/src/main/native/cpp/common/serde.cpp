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

#include "wfcore/common/serde.h"

#include "wips/pose3.wips.h"
#include "wips/apriltag_relative_pose_observation.wips.h"
#include "wips/apriltag_field_pose_observation.wips.h"
#include "wips/pipeline_result.wips.h"

#include <gtsam/geometry/Point3.h>
#include <gtsam/geometry/Rot3.h>

namespace impl {
    static wips_pose3_t wfcore2wips_pose3_shim(const gtsam::Pose3& pose) {
        auto q = pose.rotation().toQuaternion();
        wips_pose3_t wipspacket = {
            pose.x(),
            pose.y(),
            pose.z(),
            q.w(),
            q.x(),
            q.y(),
            q.z()
        };
        return wipspacket;
    }

    static gtsam::Pose3 wips2wfcore_pose3_shim(const wips_pose3_t& pose) {
        return {
            {
                pose.wq,
                pose.xq,
                pose.yq,
                pose.zq
            },
            {
                pose.x,
                pose.y,
                pose.z
            }
        };
    }

    static wips_apriltag_relative_pose_observation_t wfcore2wips_rpo_shim(const wf::ApriltagRelativePoseObservation observation) {
        return {
            observation.id,

            observation.corners[0].x,observation.corners[0].y,
            observation.corners[1].x,observation.corners[1].y,
            observation.corners[2].x,observation.corners[2].y,
            observation.corners[3].x,observation.corners[3].y,

            observation.decisionMargin,
            observation.hammingDistance,
            wfcore2wips_pose3_shim(observation.camPose0),
            observation.error0,
            wfcore2wips_pose3_shim(observation.camPose1),
            observation.error1
        };
    }

    static wf::ApriltagRelativePoseObservation wips2wfcore_rpo_shim(const wips_apriltag_relative_pose_observation_t& observation) {
        return {
            observation.fiducial_id,
            {
                {observation.corner0_x, observation.corner0_y},
                {observation.corner1_x, observation.corner1_y},
                {observation.corner2_x, observation.corner2_y},
                {observation.corner3_x, observation.corner3_y}
            },
            observation.decision_margin,
            observation.hamming_distance,
            wips2wfcore_pose3_shim(observation.cam_pose_0),
            observation.error_0,
            wips2wfcore_pose3_shim(observation.cam_pose_1),
            observation.error_1
        };
    }
        
    static wips_apriltag_field_pose_observation_t wfcore2wips_fpo_shim(const wf::ApriltagFieldPoseObservation& observation) {
        wips_i32_t* tagsUsedData = static_cast<wips_i32_t*>(malloc(observation.tagsUsed.size() * sizeof(wips_i32_t)));
        memcpy(tagsUsedData, observation.tagsUsed.data(), observation.tagsUsed.size() * sizeof(wips_i32_t));
        return {
            observation.tagsUsed.size(),
            tagsUsedData,
            wfcore2wips_pose3_shim(observation.fieldPose0),
            observation.error0,
            static_cast<wips_u8_t>(observation.fieldPose1.has_value()),
            observation.fieldPose1.has_value() ? wfcore2wips_pose3_shim(observation.fieldPose1.value()) : wips_pose3_t{},
            static_cast<wips_u8_t>(observation.error1.has_value()),
            observation.error1.has_value() ? observation.error1.value() : 0.0
        };
    }

    static wf::ApriltagFieldPoseObservation wips2wfcore_fpo_shim(const wips_apriltag_field_pose_observation_t& observation) {
        std::vector<int> tagsUsed(observation.tags_used,observation.tags_used + observation.DETAILvlasize__tags_used);
        return {
            std::vector(observation.tags_used,observation.tags_used + observation.DETAILvlasize__tags_used),
            wips2wfcore_pose3_shim(observation.field_pose_0),
            observation.error_0,
            observation.DETAILoptpresent__field_pose_1 ? std::make_optional(wips2wfcore_pose3_shim(observation.field_pose_1)) : std::nullopt,
            observation.DETAILoptpresent__error_1 ? std::make_optional(observation.error_1) : std::nullopt
        };
    }

}

namespace wf {

    wips_bin_t* packPose3(const gtsam::Pose3& pose) {
        wips_pose3_t wipspose = impl::wfcore2wips_pose3_shim(pose);
        wips_bin_t* bin = wips_bin_create(sizeof(wips_pose3_t));
        wips_encode_pose3(bin, &wipspose);
        wips_pose3_free_resources(&wipspose);
        return bin;
    }

    gtsam::Pose3 unpackPose3(wips_bin_t* data) {
        wips_pose3_t wipspose;
        wips_decode_pose3(&wipspose,data);
        auto out = impl::wips2wfcore_pose3_shim(wipspose);
        wips_pose3_free_resources(&wipspose);
        return out;
    }

    wips_bin_t* packApriltagRelativePoseObservation(const ApriltagRelativePoseObservation& poseObservation) {
        wips_apriltag_relative_pose_observation_t wipsobservation = impl::wfcore2wips_rpo_shim(poseObservation);
        wips_bin_t* bin = wips_bin_create(sizeof(wips_apriltag_relative_pose_observation_t));
        wips_encode_apriltag_relative_pose_observation(bin, &wipsobservation);
        wips_apriltag_relative_pose_observation_free_resources(&wipsobservation);
        return bin;
    }
    ApriltagRelativePoseObservation unpackApriltagRelativePoseObservation(wips_bin_t* data) {
        wips_apriltag_relative_pose_observation_t wipsobservation;
        wips_decode_apriltag_relative_pose_observation(&wipsobservation, data);
        auto out = impl::wips2wfcore_rpo_shim(wipsobservation);
        wips_apriltag_relative_pose_observation_free_resources(&wipsobservation);
        return out;
    }

    wips_bin_t* packApriltagFieldPoseObservation(const ApriltagFieldPoseObservation& poseObservation) {
        wips_apriltag_field_pose_observation_t wipsobservation = impl::wfcore2wips_fpo_shim(poseObservation);
        wips_bin_t* bin = wips_bin_create(sizeof(wips_apriltag_field_pose_observation_t));
        wips_encode_apriltag_field_pose_observation(bin, &wipsobservation);
        wips_apriltag_field_pose_observation_free_resources(&wipsobservation);
        return bin;
    }

    ApriltagFieldPoseObservation unpackApriltagFieldPoseObservation(wips_bin_t* data) {
        wips_apriltag_field_pose_observation_t wipsobservation;
        wips_decode_apriltag_field_pose_observation(&wipsobservation, data);
        auto out = impl::wips2wfcore_fpo_shim(wipsobservation);
        wips_apriltag_field_pose_observation_free_resources(&wipsobservation);
        return out;
    }

    //std::vector<unsigned char> packPipelineResult(const PipelineResult& pipelineResult);
    //PipelineResult unpackPipelineResult(const std::vector<unsigned char>& data);
}