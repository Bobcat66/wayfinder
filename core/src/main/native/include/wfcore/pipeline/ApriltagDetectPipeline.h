#pragma once

#include "wfcore/pipeline/Pipeline.h"
#include "wfcore/fiducial/ApriltagDetector.h"
#include "wfcore/fiducial/ApriltagField.h"
#include "wfcore/configuration/ApriltagConfiguration.h"

namespace wf {

    struct ApriltagDetectPipelineConfiguration {
        ApriltagDetectorConfig detConfig;
        QuadThresholdParams detQTPs;
        std::unordered_set<int> detectorExcludes;
    };
    
}