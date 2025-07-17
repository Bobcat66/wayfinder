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

#include "wips/pose3.wips.h"
#include "wips/apriltag_relative_pose_observation.wips.h"
#include "wips/apriltag_field_pose_observation.wips.h"
#include "wips/pipeline_result.wips.h"

#include <gtsam/geometry/Point3.h>
#include <gtsam/geometry/Rot3.h>
#include <unordered_map>
#include <opencv2/core.hpp>

// This file is now DEPRECATED. Serde shim code should be rewritten with WIPSSerializable

namespace impl {

    // TODO: Figure out a better way to do this
    static std::unordered_map<std::string, wips_u8_t> tagFamilyIDs = {
        {"tag36h11", 0},
        {"tag36h10", 1},
        {"tag25h9", 2},
        {"tag16h5", 3},
        {"tagCircle21h7", 4},
        {"tagCircle49h12", 5},
        {"tagCustom48h12", 6},
        {"tagStandard41h12", 7},
        {"tagStandard52h13", 8}
    };

    static std::unordered_map<wips_u8_t, std::string> tagFamilyNames = {
        {0, "tag36h11"},
        {1, "tag36h10"},
        {2, "tag25h9"},
        {3, "tag16h5"},
        {4, "tagCircle21h7"},
        {5, "tagCircle49h12"},
        {6, "tagCustom48h12"},
        {7, "tagStandard41h12"},
        {8, "tagStandard52h13"}
    };

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

    static wips_apriltag_detection_t wfcore2wips_atd_shim(const wf::ApriltagDetection& detection) {
        return {
            detection.id,
            detection.corners[0].x,detection.corners[0].y,
            detection.corners[1].x,detection.corners[1].y,
            detection.corners[2].x,detection.corners[2].y,
            detection.corners[3].x,detection.corners[3].y,
            detection.decisionMargin,
            detection.hammingDistance,
            tagFamilyIDs.at(detection.family)
        };
    }

    static wf::ApriltagDetection wips2wfcore_atd_shim(const wips_apriltag_detection_t& detection) {
        return {
            detection.fiducial_id,
            {
                cv::Point2d{detection.corner0_x, detection.corner0_y},
                cv::Point2d{detection.corner1_x, detection.corner1_y},
                cv::Point2d{detection.corner2_x, detection.corner2_y},
                cv::Point2d{detection.corner3_x, detection.corner3_y}
            },
            detection.decision_margin,
            detection.hamming_distance,
            tagFamilyNames.at(detection.tag_family_id)
        };
    }

    static wips_apriltag_relative_pose_observation_t wfcore2wips_rpo_shim(const wf::ApriltagRelativePoseObservation observation) {
        return {
            observation.id,
            wfcore2wips_pose3_shim(observation.camPose0),
            observation.error0,
            wfcore2wips_pose3_shim(observation.camPose1),
            observation.error1
        };
    }

    static wf::ApriltagRelativePoseObservation wips2wfcore_rpo_shim(const wips_apriltag_relative_pose_observation_t& observation) {
        return {
            observation.fiducial_id,
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
            static_cast<wips_u32_t>(observation.tagsUsed.size()),
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
            std::vector(
                observation.tags_used,
                observation.tags_used + GET_WIPS_DETAIL(&observation,tags_used,vlasize)
            ),
            wips2wfcore_pose3_shim(observation.field_pose_0),
            observation.error_0,
            GET_WIPS_DETAIL(&observation,field_pose_1,optpresent) 
                ? std::make_optional(wips2wfcore_pose3_shim(observation.field_pose_1)) 
                : std::nullopt,
            GET_WIPS_DETAIL(&observation,error_1,optpresent) 
                ? std::make_optional(observation.error_1) 
                : std::nullopt
        };
    }

    static wips_object_detection_t wfcore2wips_object_detection_shim(const wf::ObjectDetection& detection) {
        return {
            detection.objectClass,
            detection.confidence,
            detection.percentArea,
            detection.bboxTopLeftPixels.x,
            detection.bboxTopLeftPixels.y,
            detection.bboxBottomRightPixels.x,
            detection.bboxBottomRightPixels.y,
            detection.bboxTopLeftNorm.x,
            detection.bboxTopLeftNorm.y,
            detection.bboxBottomRightNorm.x,
            detection.bboxBottomRightNorm.y,
        };
    }

    static wf::ObjectDetection wips2wfcore_object_detection_shim(const wips_object_detection_t& detection) {
        return {
            detection.object_class,
            detection.confidence,
            detection.percent_area,
            {
                detection.topleft_x_pixels,
                detection.topleft_y_pixels
            },
            {
                detection.bottomright_x_pixels,
                detection.bottomright_y_pixels
            },
            {
                detection.topleft_x_norm,
                detection.topleft_y_norm
            },
            {
                detection.bottomright_x_norm,
                detection.bottomright_y_norm
            }
        };
    }

    static wips_pipeline_result_t wfcore2wips_pipeline_result_shim(const wf::PipelineResult& pipelineResult) {
        wips_apriltag_detection_t* detections_data 
            = static_cast<wips_apriltag_detection_t*>(
                malloc(pipelineResult.aprilTagDetections.size() * sizeof(wips_apriltag_detection_t))
            );
        wips_apriltag_relative_pose_observation_t* tag_poses_data 
            = static_cast<wips_apriltag_relative_pose_observation_t*>(
                malloc(pipelineResult.aprilTagPoses.size() * sizeof(wips_apriltag_relative_pose_observation_t))
            );
        wips_object_detection_t* object_detections_data 
            = static_cast<wips_object_detection_t*>(
                malloc(pipelineResult.objectDetections.size() * sizeof(wips_object_detection_t))
            );

        for (size_t i = 0; i < pipelineResult.aprilTagDetections.size(); ++i) {
            detections_data[i] = wfcore2wips_atd_shim(pipelineResult.aprilTagDetections[i]);
        }

        for (size_t i = 0; i < pipelineResult.aprilTagPoses.size(); ++i) {
            tag_poses_data[i] = wfcore2wips_rpo_shim(pipelineResult.aprilTagPoses[i]);
        }

        for (size_t i = 0; i < pipelineResult.objectDetections.size(); ++i) {
            object_detections_data[i] = wfcore2wips_object_detection_shim(pipelineResult.objectDetections[i]);
        }
        return {
            pipelineResult.captimeMicros,
            static_cast<wips_u8_t>(pipelineResult.type),
            static_cast<wips_u32_t>(pipelineResult.aprilTagDetections.size()),
            detections_data,
            static_cast<wips_u32_t>(pipelineResult.aprilTagPoses.size()),
            tag_poses_data,
            static_cast<wips_u8_t>(pipelineResult.cameraPose.has_value()),
            pipelineResult.cameraPose.has_value() 
                ? wfcore2wips_fpo_shim(pipelineResult.cameraPose.value()) 
                : wips_apriltag_field_pose_observation_t{},
            static_cast<wips_u32_t>(pipelineResult.objectDetections.size()),
            object_detections_data
        };
    }

    static wf::PipelineResult wips2wfcore_pipeline_result_shim(const wips_pipeline_result_t& pipelineResult) {
        std::vector<wf::ApriltagDetection> detections;
        detections.reserve(GET_WIPS_DETAIL(&pipelineResult,tag_detections,vlasize));
        for (size_t i = 0; i < GET_WIPS_DETAIL(&pipelineResult,tag_detections,vlasize); ++i) {
            detections.push_back(wips2wfcore_atd_shim(pipelineResult.tag_detections[i]));
        }
        
        std::vector<wf::ApriltagRelativePoseObservation> tagPoses;
        tagPoses.reserve(GET_WIPS_DETAIL(&pipelineResult,tag_poses,vlasize));
        for (size_t i = 0; i < GET_WIPS_DETAIL(&pipelineResult,tag_poses,vlasize); ++i) {
            tagPoses.push_back(wips2wfcore_rpo_shim(pipelineResult.tag_poses[i]));
        }

        std::vector<wf::ObjectDetection> objectDetections;
        for (size_t i = 0; i < GET_WIPS_DETAIL(&pipelineResult,object_detections,vlasize); ++i) {
            objectDetections.push_back(wips2wfcore_object_detection_shim(pipelineResult.object_detections[i]));
        }

        return {
            pipelineResult.timestamp,
            static_cast<wf::PipelineType>(pipelineResult.pipeline_type),
            std::move(detections),
            std::move(tagPoses),
            GET_WIPS_DETAIL(&pipelineResult,field_pose,optpresent)
                ? std::make_optional(wips2wfcore_fpo_shim(pipelineResult.field_pose))
                : std::nullopt,
            std::move(objectDetections)
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

    wips_bin_t* packApriltagDetection(const ApriltagDetection& detection) {
        wips_apriltag_detection_t wipsdetection = impl::wfcore2wips_atd_shim(detection);
        wips_bin_t* bin = wips_bin_create(sizeof(wips_apriltag_detection_t));
        wips_encode_apriltag_detection(bin, &wipsdetection);
        wips_apriltag_detection_free_resources(&wipsdetection);
        return bin;
    }
    ApriltagDetection unpackApriltagDetection(wips_bin_t* data) {
        wips_apriltag_detection_t wipsdetection;
        wips_decode_apriltag_detection(&wipsdetection, data);
        auto out = impl::wips2wfcore_atd_shim(wipsdetection);
        wips_apriltag_detection_free_resources(&wipsdetection);
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

    wips_bin_t* packObjectDetection(const ObjectDetection& detection) {
        wips_object_detection_t wipsdetection = impl::wfcore2wips_object_detection_shim(detection);
        wips_bin_t* bin = wips_bin_create(sizeof(wips_object_detection_t));
        wips_encode_object_detection(bin, &wipsdetection);
        wips_object_detection_free_resources(&wipsdetection);
        return bin;
    }
    ObjectDetection unpackObjectDetection(wips_bin_t* data) {
        wips_object_detection_t wipsdetection;
        wips_decode_object_detection(&wipsdetection, data);
        auto out = impl::wips2wfcore_object_detection_shim(wipsdetection);
        wips_object_detection_free_resources(&wipsdetection);
        return out;
    }

    wips_bin_t* packPipelineResult(const PipelineResult& pipelineResult) {
        wips_pipeline_result_t wipspipeline = impl::wfcore2wips_pipeline_result_shim(pipelineResult);
        wips_bin_t* bin = wips_bin_create(sizeof(wips_pipeline_result_t));
        wips_encode_pipeline_result(bin, &wipspipeline);
        wips_pipeline_result_free_resources(&wipspipeline);
        return bin;
    }
    PipelineResult unpackPipelineResult(wips_bin_t* data) {
        wips_pipeline_result_t wipspipelineresult;
        wips_decode_pipeline_result(&wipspipelineresult, data);
        auto out = impl::wips2wfcore_pipeline_result_shim(wipspipelineresult);
        wips_pipeline_result_free_resources(&wipspipelineresult);
        return out;
    }
}