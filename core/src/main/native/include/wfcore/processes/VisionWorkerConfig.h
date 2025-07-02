#pragma once

#include <string>
#include <optional>
#include "wfcore/pipeline/Pipeline.h"
#include "wfcore/pipeline/ApriltagPipeline.h"
#include "wfcore/pipeline/ObjectDetectionPipeline.h"
#include "wfcore/pipeline/ApriltagDetectPipeline.h"
#include "wfcore/video/video_types.h"
namespace wf {
    struct VisionWorkerConfig {
        std::string devpath;
        std::string name;
        StreamFormat inputFormat; // Format of the input stream
        StreamFormat outputFormat; // Format of the output stream
        bool stream;
        int raw_port;
        int processed_port;
        std::optional<ApriltagPipelineConfiguration> apriltagPipelineConfig;
        std::optional<ObjectDetectionPipelineConfiguration> objectDetectionPipelineConfig;
        std::optional<ApriltagDetectPipelineConfiguration> apriltagDetectPipelineConfig;
    };
}