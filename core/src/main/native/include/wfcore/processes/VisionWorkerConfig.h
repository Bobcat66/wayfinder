#pragma once

#include <string>
#include <variant>
#include "wfcore/pipeline/Pipeline.h"
#include "wfcore/pipeline/ApriltagPipeline.h"
#include "wfcore/pipeline/ObjectDetectionPipeline.h"
#include "wfcore/video/video_types.h"
#include "wfcore/common/json_utils.h"

namespace wf {
    struct VisionWorkerConfig : public JSONSerializable<VisionWorkerConfig> {
        std::string camera_nickname;
        std::string name;
        StreamFormat inputFormat; // Format of the input stream
        StreamFormat outputFormat; // Format of the output stream
        bool stream;
        int raw_port;
        int processed_port;
        PipelineType pipelineType;
        std::variant<
            ApriltagPipelineConfiguration,
            ObjectDetectionPipelineConfiguration
        > pipelineConfig;

        VisionWorkerConfig(
            std::string camera_nickname_, std::string name_,
            StreamFormat inputFormat_, StreamFormat outputFormat_,
            bool stream_,
            int raw_port_, int processed_port_,
            PipelineType pipelineType_,
            std::variant<
                ApriltagPipelineConfiguration,
                ObjectDetectionPipelineConfiguration
            > pipelineConfig_
        ) : camera_nickname(std::move(camera_nickname_)), name(std::move(name_))
        , inputFormat(std::move(inputFormat_)), outputFormat(std::move(outputFormat_))
        , stream(stream_), raw_port(raw_port_), processed_port(processed_port_)
        , pipelineConfig(std::move(pipelineConfig_)) {}

        static WFResult<VisionWorkerConfig> fromJSON_impl(const JSON& jobject);
        static WFResult<JSON> toJSON_impl(const VisionWorkerConfig& config);
        static const jval::JSONValidationFunctor* getValidator_impl();
    };
}