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

#pragma once

#include "wfcore/pipeline/Pipeline.h"
#include "wfcore/fiducial/ApriltagDetector.h"
#include "wfcore/fiducial/ApriltagField.h"
#include "wfcore/fiducial/ApriltagConfiguration.h"
#include "wfcore/hardware/CameraConfiguration.h"
#include "wfcore/common/json_utils.h"
#include "wfcore/pipeline/ApriltagPipelineConfiguration.h"

namespace wf {

    class ApriltagPipeline : public Pipeline {
    public:
        ApriltagPipeline(ApriltagPipelineConfiguration config_, CameraIntrinsics intrinsics_, ApriltagConfiguration tagConfig_, ApriltagField& tagField_);
        WFStatusResult setConfig(const ApriltagPipelineConfiguration& config);
        WFStatusResult setTagConfig(const ApriltagConfiguration& tagConfig);
        void setTagField(const ApriltagField& tagField);
        void setIntrinsics(const CameraIntrinsics& intrinsics);
        [[nodiscard]] 
        WFResult<PipelineResult> process(const cv::Mat& data, const FrameMetadata& meta) noexcept override;
        ~ApriltagPipeline() override = default;
    private:
        WFStatusResult updateDetectorConfig(); // Updates the apriltag detector's configuration
        ApriltagPipelineConfiguration config;
        CameraIntrinsics intrinsics;
        ApriltagConfiguration tagConfig;
        ApriltagField& tagField;
        ApriltagDetector detector;
    };
}