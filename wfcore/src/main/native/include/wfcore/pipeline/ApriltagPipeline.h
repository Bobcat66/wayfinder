// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "wfcore/pipeline/Pipeline.h"
#include "wfcore/fiducial/ApriltagDetector.h"
#include "wfcore/fiducial/ApriltagField.h"

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
        ApriltagPipeline(ApriltagPipelineConfiguration config_, CameraIntrinsics intrinsics_, ApriltagField field_);
        [[nodiscard]] 
        PipelineResult process(const Frame& frame) const noexcept override; // TODO, make this override explicit when an implementation exists
        ~ApriltagPipeline() override = default;
    private:
        ApriltagPipelineConfiguration config;
        CameraIntrinsics intrinsics;
        ApriltagField field;
        ApriltagDetector detector;
    };
}