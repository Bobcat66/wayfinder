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
#include <opencv2/core.hpp>

namespace wf {
    // Parameters for how the inference engine should tensorize the input frame
    // Most image preprocessing (cropping, resizing, color space conversion) is assumed to be done outside of this class
    struct TensorParameters {
        bool interleaved = false; // If true, the tensor image data will be in interleaved format (e.g. HWC/NHWC), otherwise it will be in planar format (e.g. CHW/NCHW)
        int height = 0; // Height of the tensor, in pixels. This is to correctly allocate buffers for the tensorization process, images are expected to already be in this size. Improperly sized images will result in undefined behavior
        int width = 0; // Width of the tensor, in pixels. This is to correctly allocate buffers for the tensorization process, images are expected to already be in this size. Improperly sized images will result in undefined behavior
        int channels = 0; // number of color channels in the image
        float scale = 1.0; // Scale factor to multiply each pixel value by. Set to 1.0 to disable normalization
        cv::Scalar stds = {1.0, 1.0, 1.0}; // Standard deviations to divide each channel by. Set to 1.0 for each channel to disable standardization
        cv::Scalar means = {0.0, 0.0, 0.0}; // Means to subtract from each channel. Set to 0.0 for each channel to disable mean centering
    };

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
        Unknown
    };

    enum class ModelArch {
        YOLO,
        SSD, // Not implemented
        RETINA_NET, // Not implemented
        RCNN, // Not implemented
        Unknown
    };
}