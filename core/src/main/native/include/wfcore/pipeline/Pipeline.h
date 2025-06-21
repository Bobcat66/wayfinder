/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of Wayfinder.
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

#pragma once

#include "wfcore/types.h"
#include "wfcore/video/video_types.h"
#include "wfcore/fiducial/ApriltagDetector.h"
#include "wfcore/pipeline/pnp.h"
#include "wfcore/inference/InferenceEngine.h"
#include <vector>
#include <optional>

namespace wf {
    /* 
     * Potential pipeline types to add:
     * Keypoint (pose estimation based on keypoints)
     * OptimizedApriltag (apriltag pose estimation based on a dynamic field optimized with a TagSLAM algorithm, requires a SLAM server somewhere on the network)
     * Depth (for depth cameras)
     * MonoSFM (local single camera Structure from Motion)
     * MonoSLAM (local single camera Simultaneous Mapping And Localization)
     */
    enum class PipelineType {
        Apriltag,
        ApriltagDetect, // This is for a pipeline that only detects apriltags, without solving PnP
        ObjDetect
    };

    struct PipelineResult {
        PipelineType type;
        std::vector<ApriltagRelativePoseObservation> aprilTagPoses;
        std::optional<ApriltagFieldPoseObservation> cameraPose;
        std::vector<ObjectDetection> objectDetections;
        Frame frame;

        PipelineResult() = default;
        PipelineResult(PipelineResult&&) = default;
        PipelineResult& operator=(PipelineResult&&) = default;
        PipelineResult(const PipelineResult&) = default;
        PipelineResult& operator=(const PipelineResult&) = default;

        PipelineResult(
            PipelineType type_,
            std::vector<ApriltagRelativePoseObservation> aprilTagPoses_,
            std::optional<ApriltagFieldPoseObservation> cameraPose_,
            std::vector<ObjectDetection> objectDetections_,
            Frame frame_
        ) 
        : type(type_), aprilTagPoses(std::move(aprilTagPoses_))
        , cameraPose(std::move(cameraPose_)), objectDetections(std::move(objectDetections_))
        , frame(std::move(frame_)) {}
        
        static PipelineResult ApriltagPipelineResult(
            std::vector<ApriltagRelativePoseObservation> aprilTagPoses_,
            std::optional<ApriltagFieldPoseObservation> cameraPose_,
            Frame frame_
        ) {
            return PipelineResult(
                PipelineType::Apriltag,
                std::move(aprilTagPoses_),
                std::move(cameraPose_),
                {},
                std::move(frame_)
            );
        }
    };

    struct PipelineConfiguration {
        std::string name;
        PipelineType type;  
    };

    class Pipeline {
    public:
        [[nodiscard]] 
        virtual PipelineResult process(const Frame& frame) const noexcept = 0;
        virtual ~Pipeline() = default;
    };
}
