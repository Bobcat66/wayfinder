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
#include "wfcore/common/logging.h"
#include <algorithm>
#include <wfcore/pipeline/pnp.h>
#include <cassert>


namespace wf {

    static loggerPtr logger = LoggerManager::getInstance().getLogger("ApriltagPipeline");

    ApriltagPipeline::ApriltagPipeline(ApriltagPipelineConfiguration config_, CameraIntrinsics intrinsics_, ApriltagConfiguration tagConfig_, ApriltagField& tagField_)
    : config(std::move(config_)), intrinsics(std::move(intrinsics_)), tagConfig(tagConfig_), tagField(tagField_) {
        updateDetectorConfig();
    }

    bool ApriltagPipeline::setConfig(const ApriltagPipelineConfiguration& config) {
        this->config = config;
        return updateDetectorConfig();
    }
    
    bool ApriltagPipeline::setTagConfig(const ApriltagConfiguration& tagConfig) {
        this->tagConfig = tagConfig;
        return updateDetectorConfig();
    }

    bool ApriltagPipeline::setTagField(const ApriltagField& tagField) {
        this->tagField = tagField;
        return true;
    }

    bool ApriltagPipeline::setIntrinsics(const CameraIntrinsics& intrinsics) {
        this->intrinsics = intrinsics;
        return true;
    }

    // TODO: Refactor this???
    bool ApriltagPipeline::updateDetectorConfig() {     
        detector.setQuadThresholdParams(config.detQTPs);
        detector.setConfig(config.detConfig);
        detector.clearFamilies();

        if (!detector.addFamily(tagConfig.tagFamily)) {
            switch (detector.getStatus()) {
                case ApriltagDetectorStatus::NullDetector:
                    this->reportError(
                        PipelineStatus::FailedResourceAcquisition,
                        detector.getError().value()
                    );
                    break;
                case ApriltagDetectorStatus::NullFamily:
                    this->reportError(
                        PipelineStatus::FailedResourceAcquisition,
                        detector.getError().value()
                    );
                    break;
                case ApriltagDetectorStatus::InvalidFamily:
                    this->reportError(
                        PipelineStatus::InvalidConfiguration,
                        detector.getError().value()
                    );
                    break;
                default:
                    this->reportError(
                        PipelineStatus::ApriltagDetectorError,
                        detector.getError().value_or("An unknown error occurred")
                    );
            }
            return false;
        }
        return true;
    }

    PipelineResult ApriltagPipeline::process(const cv::Mat& data, const FrameMetadata& meta) noexcept {
        assert(data.type() == CV_8UC1);
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
            tagField,
            intrinsics,
            config.SolvePNPExcludes
        );
        return PipelineResult::ApriltagResult(
            meta.micros,
            std::move(detections),
            std::move(atagPoses),
            std::move(fieldPose)
        );
    }

}

