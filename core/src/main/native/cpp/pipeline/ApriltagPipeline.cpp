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


#include "wfcore/pipeline/ApriltagPipeline.h"
#include "wfcore/hardware/CameraConfiguration.h"
#include <algorithm>
#include <wfcore/pipeline/pnp.h>

namespace wf {
    ApriltagPipeline::ApriltagPipeline(ApriltagPipelineConfiguration config_, CameraIntrinsics intrinsics_, ApriltagConfiguration tagConfig_)
    : config(std::move(config_)), intrinsics(std::move(intrinsics_)), tagConfig(tagConfig_) {
        updateDetectorConfig();
    }

    void ApriltagPipeline::setConfig(const ApriltagPipelineConfiguration& config) {
        this->config = config;
        updateDetectorConfig();
    }
    
    void ApriltagPipeline::setTagConfig(const ApriltagConfiguration& tagConfig) {
        this->tagConfig = tagConfig;
        updateDetectorConfig();
    }

    void ApriltagPipeline::setIntrinsics(const CameraIntrinsics& intrinsics) {
        this->intrinsics = intrinsics;
    }

    void ApriltagPipeline::updateDetectorConfig() {
        detector.setQuadThresholdParams(config.detQTPs);
        detector.setConfig(config.detConfig);
        detector.clearFamilies();
        detector.addFamily(tagConfig.tagFamily);
    }

    PipelineResult ApriltagPipeline::process(const cv::Mat& data, const FrameMetadata& meta) const noexcept {
        auto detections = detector.detect(data);
        std::erase_if(detections, [this](ApriltagDetection detection) {
            return this->config.detectorExcludes.contains(detection.id);
        });
        std::vector<ApriltagRelativePoseObservation> atagPoses;
        if (config.solveTagRelative) {
            for (auto detection : detections) {
                auto atagPose = solvePNPApriltagRelative(
                    detection,
                    tagConfig,
                    intrinsics
                );
                if (atagPose.has_value()) {
                    atagPoses.push_back(atagPose.value());
                }
            }
        }
        auto fieldPose = solvePNPApriltag(
            detections,
            tagConfig,
            intrinsics,
            config.SolvePNPExcludes
        );
        return PipelineResult::ApriltagPipelineResult(
            meta.micros,
            detections,
            atagPoses,
            fieldPose
        );
    }

}

