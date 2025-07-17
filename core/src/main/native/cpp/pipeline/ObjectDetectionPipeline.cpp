
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

#include "wfcore/pipeline/ObjectDetectionPipeline.h"
#include "wfcore/common/wfexcept.h"
#include "wfcore/inference/CPUInferenceEngineYOLO.h"

namespace wf {
    std::unique_ptr<InferenceEngine> ObjectDetectionPipeline::buildInferenceEngine(const ObjectDetectionPipelineConfiguration& config) {
        using enum InferenceEngineType;
        using enum ModelArch;
        switch (config.engineType) {
            case CV_CPU:
                switch (config.modelArch) {
                    case YOLO: {
                        auto engine = std::make_unique<CPUInferenceEngineYOLO>();
                        engine->setFilteringParameters(config.filterParams);
                        engine->setTensorParameters(config.tensorParams);
                        if (!engine->loadModel(config.modelPath)) {
                            throw model_not_loaded();
                        }
                        return std::move(engine);
                    }
                    case SSD: throw invalid_model_arch("SSD models are not supported on CV CPU inference engines");
                    case RETINA_NET: throw invalid_model_arch("RETINA_NET models are not supported on CV CPU inference engines");
                    case RCNN: throw invalid_model_arch("RCNN models are not supported on CV CPU inference engines");
                    default: throw invalid_model_arch("Unknown model architecture specified. How did you even do this?");
                }
            case CV_OPENCL: throw invalid_engine_type("CV OpenCL inference engines are not supported at the moment.");
            case CV_VULKAN: throw invalid_engine_type("CV Vulkan inference engines are not supported at the moment.");
            case CUDA: throw invalid_engine_type("CUDA inference engines are not supported at the moment.");
            case OpenVINO: throw invalid_engine_type("OpenVINO inference engines are not supported at the moment.");
            case RKNN: throw invalid_engine_type("RKNN inference engines are not supported at the moment.");
            case CoreML: throw invalid_engine_type("CoreML inference engines are not supported at the moment.");
            case ROCm: throw invalid_engine_type("ROCm inference engines are not supported at the moment.");
            case EdgeTPU: throw invalid_engine_type("EdgeTPU inference engines are not supported at the moment.");
            case HailoRT: throw invalid_engine_type("HailoRT inference engines are not supported at the moment.");
            default: throw invalid_engine_type("Unknown engine type specified. How did you even do this?");
        }
    }
    ObjectDetectionPipeline::ObjectDetectionPipeline(ObjectDetectionPipelineConfiguration config_, CameraIntrinsics intrinsics_) 
    : config(std::move(config_)), intrinsics(std::move(intrinsics_)) {
        engine = buildInferenceEngine(config);
        updatePostprocParams();
    }
    PipelineResult ObjectDetectionPipeline::process(const cv::Mat& data, const FrameMetadata& meta) noexcept {
        assert(data.rows == tens)
    }
    void ObjectDetectionPipeline::updatePostprocParams() {
        const auto& tensorParams = engine->getTensorParameters();
        scale = 1/(std::min(
            static_cast<double>(tensorParams.width)/intrinsics.resolution.width,
            static_cast<double>(tensorParams.height)/intrinsics.resolution.height
        )); // Reciprocal of the scaling factor used to letterbox the image in the first place

        const int realWidth = static_cast<int>(intrinsics.resolution.width/scale); // The width of the actual image within the letterboxed image
        const int realHeight = static_cast<int>(intrinsics.resolution.height/scale); // The height of the actual image within the letterboxed image
        horizontalShift = -((intrinsics.resolution.width - realWidth)/2);
        verticalShift = -((intrinsics.resolution.height - realHeight)/2);
    }
}
