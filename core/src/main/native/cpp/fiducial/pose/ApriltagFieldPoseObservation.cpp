/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 Jesse Kane
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

#include "wfcore/fiducial/pose/ApriltagFieldPoseObservation.h"
#include "wfcore/common/serde/shims.h"

namespace wf {
    wips_apriltag_field_pose_observation_t ApriltagFieldPoseObservation::toWIPS_impl(const ApriltagFieldPoseObservation& observation) {
        wips_i32_t* tagsUsedData = static_cast<wips_i32_t*>(malloc(observation.tagsUsed.size() * sizeof(wips_i32_t)));
        memcpy(tagsUsedData, observation.tagsUsed.data(), observation.tagsUsed.size() * sizeof(wips_i32_t));
        return {
            static_cast<wips_u32_t>(observation.tagsUsed.size()),
            tagsUsedData,
            pose3_wfcore2wips(observation.fieldPose0),
            observation.error0,
            static_cast<wips_u8_t>(observation.fieldPose1.has_value()),
            observation.fieldPose1.has_value() ? pose3_wfcore2wips(observation.fieldPose1.value()) : wips_pose3_t{},
            static_cast<wips_u8_t>(observation.error1.has_value()),
            observation.error1.has_value() ? observation.error1.value() : 0.0
        };
    }
    ApriltagFieldPoseObservation ApriltagFieldPoseObservation::fromWIPS_impl(const wips_apriltag_field_pose_observation_t& observation) {
        return {
            std::vector(
                observation.tags_used,
                observation.tags_used + GET_WIPS_DETAIL(&observation,tags_used,vlasize)
            ),
            pose3_wips2wfcore(observation.field_pose_0),
            observation.error_0,
            GET_WIPS_DETAIL(&observation,field_pose_1,optpresent) 
                ? std::make_optional(pose3_wips2wfcore(observation.field_pose_1)) 
                : std::nullopt,
            GET_WIPS_DETAIL(&observation,error_1,optpresent) 
                ? std::make_optional(observation.error_1) 
                : std::nullopt
        };
    }
}