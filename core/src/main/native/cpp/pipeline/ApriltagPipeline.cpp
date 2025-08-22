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
#include "wfcore/common/wfexcept.h"


namespace wf {

    static loggerPtr logger = LoggerManager::getInstance().getLogger("ApriltagPipeline");

    ApriltagPipeline::ApriltagPipeline(ApriltagPipelineConfiguration config_, CameraIntrinsics intrinsics_, ApriltagFieldHandler fieldHandler_)
    : config(std::move(config_)), intrinsics(std::move(intrinsics_)), tagConfig(config.apriltagFamily,config.apriltagSize), fieldHandler(std::move(fieldHandler_)) {
        auto fres = updateFieldHandler();
        if (!fres) throw wf_result_error(fres);
        
        auto dres = updateDetectorConfig();
        if (!dres) throw wf_result_error(dres);
    }

    WFStatusResult ApriltagPipeline::setConfig(const ApriltagPipelineConfiguration& config) {
        this->config = config;
        return updateDetectorConfig();
    }
    
    void ApriltagPipeline::setIntrinsics(const CameraIntrinsics& intrinsics) {
        this->intrinsics = intrinsics;
    }

    WFStatusResult ApriltagPipeline::updateFieldHandler() {
        if (fieldHandler.getFieldName() == config.apriltagField) 
            return WFStatusResult::success();

        return fieldHandler.loadField(config.apriltagField);
    }

    // TODO: Refactor this???
    WFStatusResult ApriltagPipeline::updateDetectorConfig() {     
        detector.clearFamilies();

        auto res = detector.addFamily(tagConfig.tagFamily);
        if (!res) return res;

        detector.setQuadThresholdParams(config.detQTPs);
        detector.setConfig(config.detConfig);
        return WFStatusResult::success();
    }

    WFResult<PipelineResult> ApriltagPipeline::process(const cv::Mat& data, const FrameMetadata& meta) noexcept {
        WF_FatalAssert(data.type() == CV_8UC1);
        auto detectres = detector.detect(data);
        if (!detectres)
            return WFResult<PipelineResult>::propagateFail(detectres);
        
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
            fieldHandler.getField(),
            intrinsics,
            config.SolvePNPExcludes
        );
        return PipelineResult::ApriltagResult(
            meta.micros,
            meta.server_time_us,
            std::move(detections),
            std::move(atagPoses),
            std::move(fieldPose)
        );
    }

}

