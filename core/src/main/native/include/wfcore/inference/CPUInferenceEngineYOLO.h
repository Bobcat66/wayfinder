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

#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <string>
#include <vector>

// A simple reference implementation of a CPU inference engine, using OpenCV's DNN module.
namespace wf {

    class CPUInferenceEngineYOLO : public InferenceEngine {
    public:
        CPUInferenceEngineYOLO();
        std::string modelFormat() const override { return "onnx"; }
        bool setFilteringParameters(const IEFilteringParams& params) override { this->filterParams = params; return true; }
        bool setTensorParameters(const TensorParameters& params) override;
        bool loadModel(const std::string& modelPath) override;
        bool infer(const cv::Mat& data, const FrameMetadata& meta, std::vector<RawBbox>& output) noexcept override;
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