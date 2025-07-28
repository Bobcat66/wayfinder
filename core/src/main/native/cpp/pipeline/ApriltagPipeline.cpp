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
#include "wfcore/common/wfassert.h"
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

    WFStatusResult ApriltagPipeline::setConfig(const ApriltagPipelineConfiguration& config) {
        this->config = config;
        return updateDetectorConfig();
    }
    
    WFStatusResult ApriltagPipeline::setTagConfig(const ApriltagConfiguration& tagConfig) {
        this->tagConfig = tagConfig;
        return updateDetectorConfig();
    }

    void ApriltagPipeline::setTagField(const ApriltagField& tagField) {
        this->tagField = tagField;
    }

    void ApriltagPipeline::setIntrinsics(const CameraIntrinsics& intrinsics) {
        this->intrinsics = intrinsics;
    }

    // TODO: Refactor this???
    WFStatusResult ApriltagPipeline::updateDetectorConfig() {     
        detector.setQuadThresholdParams(config.detQTPs);
        detector.setConfig(config.detConfig);
        detector.clearFamilies();

        if (auto detres = detector.addFamily(tagConfig.tagFamily)) {
            return WFStatusResult::success();
        } else {
            return WFStatusResult::propagateFail(detres);
        }
    }

    PipelineResult ApriltagPipeline::process(const cv::Mat& data, const FrameMetadata& meta) noexcept {
        WF_Assert(data.type() == CV_8UC1);
        auto detectres = detector.detect(data);
        if (!detectres){
            this->reportError(detectres);
            return PipelineResult::NullResult();
        }
        auto detections = std::move(detectres.value());
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
        this->reportOk();
        return PipelineResult::ApriltagResult(
            meta.micros,
            std::move(detections),
            std::move(atagPoses),
            std::move(fieldPose)
        );
    }

}

