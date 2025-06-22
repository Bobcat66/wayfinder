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

#include "wfcore/pipeline/Pipeline.h"
#include "wfcore/fiducial/ApriltagDetector.h"
#include "wfcore/fiducial/ApriltagField.h"
#include "wfcore/configuration/ApriltagConfiguration.h"

namespace wf {

    struct ApriltagPipelineConfiguration {
        ApriltagDetectorConfig detConfig;
        QuadThresholdParams detQTPs;
        std::unordered_set<int> detectorExcludes;
        std::unordered_set<int> SolvePNPExcludes; // Does not effect tag relative solvePNP
        bool solveTagRelative; // Whether or not to solve tag relative
    };

    class ApriltagPipeline : public Pipeline {
    public:
        ApriltagPipeline(ApriltagPipelineConfiguration config_, CameraIntrinsics intrinsics_, ApriltagConfiguration tagConfig_);
        [[nodiscard]] 
        PipelineResult process(const Frame& frame) const noexcept override;
        ~ApriltagPipeline() override = default;
    private:
        ApriltagPipelineConfiguration config;
        CameraIntrinsics intrinsics;
        ApriltagConfiguration tagConfig;
        ApriltagDetector detector;
    };
}