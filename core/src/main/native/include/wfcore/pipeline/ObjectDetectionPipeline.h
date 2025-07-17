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
#include "wfcore/inference/Tensorizer.h"
#include "wfcore/hardware/CameraConfiguration.h"

#include <string>
#include <memory>
#include <vector>

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
        ModelArch modelArch; // Architecture of the model
        InferenceEngineType engineType; // Type of inference engine to use
        TensorParameters tensorParams; // Parameters for the tensorizer. The resolution and format of images input to the model should match the tensor parameters, NOT the native resolution
        ImageEncoding modelColorSpace; // Color space the model expects pixels to be in
        IEFilteringParams filterParams;
    };

    class ObjectDetectionPipeline : public Pipeline {
    public:
        ObjectDetectionPipeline(ObjectDetectionPipelineConfiguration config_, CameraIntrinsics intrinsics_);
        [[nodiscard]] 
        PipelineResult process(const cv::Mat& data, const FrameMetadata& meta) noexcept override;
        [[nodiscard]]
        PipelineStatus getStatus() const noexcept override { return status; }
        [[nodiscard]]
        std::string getStatusMsg() const noexcept override { return statusMsg; }
        InferenceEngineType getEngineType() {return config.engineType;}
        ImageEncoding getModelColorSpace() {return config.modelColorSpace;}
        static std::unique_ptr<InferenceEngine> buildInferenceEngine(const ObjectDetectionPipelineConfiguration& config);
    private:
        void updatePostprocParams();
        // TODO: Implement these
        PipelineStatus status;
        std::string statusMsg;
        std::unique_ptr<InferenceEngine> engine;
        ObjectDetectionPipelineConfiguration config;
        std::vector<RawBbox> bbox_buffer;
        std::vector<cv::Point2d> pixelCorner_buffer; // Even indexed points are topleft corners, odd indexed points are bottomright corners.
        std::vector<cv::Point2d> normCorner_buffer;

        // This is for calculating corner angles, should be calibrated for the native input resolution
        CameraIntrinsics intrinsics;
        // Postproc params
        // These are dynamically calculated based on the tensor parameters and the native resolution,
        // essentially running the same algorithm used by the LetterboxNode to compute the padding and scale factors, but in reverse
        double horizontalShift; // -leftPadding
        double verticalShift; // -topPadding
        double scale;
    };
}
