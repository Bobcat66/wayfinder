// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "wfcore/pipeline/Pipeline.h"
#include "wfcore/fiducial/ApriltagDetector.h"

namespace wf {

    class ApriltagPipeline : public Pipeline {
        public:
        PipelineResult process(const cv::Mat& frame) const; // TODO, make this override explicit when an implementation exists
    };
}