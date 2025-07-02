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
#include "wfcore/inference/InferenceEngine.h"
#include "wfcore/video/video_types.h"

#include <string>

namespace wf {

    // All AI stuff is designed to work with YOLO models
    enum class InferenceEngineType {
        CV_CPU, // OpenCV DNN-based inference engine
        CV_OPENCL, // OpenCV-based inference engine with OpenCL acceleration (not implemented yet)
        CV_VULKAN, // OpenCV-based inference engine with Vulkan acceleration (not implemented yet)
        CUDA, // NVIDIA TensorRT-based inference engine, uses CUDA terminology for better brand recognition (not implemented yet)
        OpenVINO, // Intel OpenVINO-based inference engine (not implemented yet)
        RKNN, // Rockchip NPU-based inference engine (not implemented yet)
        CoreML, // Apple Core ML-based inference engine (not implemented yet)
        ROCm, // AMD MIVisionX-based inference engine, uses ROCm terminology for better brand recognition (not implemented yet),
        EdgeTPU, // Google Edge TPU-based inference engine (not implemented yet)
        HailoRT, // Hailo RT-based inference engine (not implemented yet)
    };

    enum class ModelArch {
        YOLO,
        SSD, // Not implemented
        RETINA_NET, // Not implemented
        RCNN // Not implemented
    };

    struct ObjectDetectionPipelineConfiguration {
        std::string modelPath; // Path to the model file
        InferenceEngineType engineType; // Type of inference engine to use
        FrameFormat modelInputFormat; // Input format of the model
        std::vector<double> means; // Mean values to subtract from the input image. If the image is in BGR format, these should be in BGR order, likewise for RGB. If grayscale, this should have only one value
        float confidenceThreshold = 0.5; // Minimum confidence threshold for detections
        float nmsThreshold = 0.4; // Non-maximum suppression threshold
    };

    class ObjectDetectionPipeline : public Pipeline {
    public:
        ObjectDetectionPipeline(ObjectDetectionPipelineConfiguration config_, CameraIntrinsics intrinsics_);
        [[nodiscard]] 
        PipelineResult process(const Frame& frame) const noexcept override;
    private:
        InferenceEngine& engine;
        CameraIntrinsics intrinsics;
        ObjectDetectionPipelineConfiguration config;
    };
}
