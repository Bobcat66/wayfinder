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

#include "wfcore/hardware/CameraConfiguration.h"
#include "wfcore/inference/InferenceEngine.h"
#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <string>
#include <vector>
#include <memory>

// A simple reference implementation of a CPU inference engine, using OpenCV's DNN module.
namespace wf {

    class CPUInferenceEngineYOLO : public InferenceEngine {
    public:
        CPUInferenceEngineYOLO(std::filesystem::path modelPath_, TensorParameters tensorParams_, IEFilteringParams filterParams);
        const std::string& modelFormat() const override {
            static const std::string format("onnx");
            return format;
        }
        void setFilteringParameters(IEFilteringParams params) override { this->filterParams = std::move(params); }
        void setTensorParameters(TensorParameters params) override;
        WFStatusResult infer(const cv::Mat& data, const FrameMetadata& meta, std::vector<RawBbox>& output) noexcept override;
        static WFResult<std::unique_ptr<CPUInferenceEngineYOLO>> creator_impl(
            std::filesystem::path modelPath,
            TensorParameters tensorParams,
            IEFilteringParams filterParams
        );
        InferenceEngineType getEngineType() const noexcept override { return InferenceEngineType::CV_CPU; }
        ModelArch getModelArch() const noexcept override { return ModelArch::YOLO; }
    private:
        cv::dnn::Net model; // OpenCV DNN network
        cv::Mat blob; // Blob for input data
        std::vector<cv::Point2f> corners_buffer;
        std::vector<cv::Point2f> norm_corners_buffer;
        std::vector<int> index_buffer;
        std::vector<cv::Rect2d> bboxd_buffer;
        std::vector<float> confidence_buffer;
        std::vector<int> objclass_buffer;
    };
}