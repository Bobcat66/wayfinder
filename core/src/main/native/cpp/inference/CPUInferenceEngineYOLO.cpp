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

#include "wfcore/inference/CPUInferenceEngineYOLO.h"
#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>
#include <cassert>

namespace wf {
    bool CPUInferenceEngineYOLO::setTensorParameters(const TensorParameters& params) {
        this->tensorizer.setTensorParameters(params);
        int dims[] = {
            1,
            params.interleaved ? params.height : params.channels,
            params.interleaved ? params.width : params.height,
            params.interleaved ? params.channels : params.width
        };
        blob = cv::Mat(4, dims, CV_32F);
        CV_Assert(blob.isContinuous());
        return true;
    }
    bool CPUInferenceEngineYOLO::loadModel(const std::string& modelPath) {
        model = cv::dnn::readNetFromONNX(modelPath);
        model.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        model.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        return !model.empty();
        return true; // TODO: Check if model is loaded correctly
    }
    [[nodiscard]] 
    std::vector<ObjectDetection> CPUInferenceEngineYOLO::infer(const Frame& input) noexcept {
        this->tensorizer.tensorize(input.data, reinterpret_cast<float*>(blob.data));
        model.setInput(blob);
        cv::Mat output = model.forward();

        int numDetections = output.rows;
        std::vector<ObjectDetection> detections;
        detections.reserve(numDetections);
        for (int i = 0; i < numDetections; ++i) {
            float* data = output.ptr<float>(i);
            int classId = static_cast<int>(data[0]);
            float confidence = data[1];
            float x1 = data[2];
            float y1 = data[3];
            float x2 = data[4];
            float y2 = data[5];
        }
    }

    std::vector<ObjectDetection> CPUInferenceEngineYOLO::postprocess(const cv::Mat& detmat) {
        int num_classes = detmat.cols - 5;
        std::vector<ObjectDetection> results;
        results.reserve(detmat.rows);
        for (int i = 0; i < detmat.rows ++i) {
            float* data = detnat.ptr<float>(i);
            float objectness = data[4];
            cv::Mat scores(1, num_classes, CV_32FC1, data + 5);
            cv::Point2f classIdPoint;
            double maxClassScore;
            double xcenter = 
            cv::minMaxLoc(scores, 0, &maxClassScore, 0, &classIdPoint);
            int objectClass = (int)classIdPoint.x;
            results.emplace_back(
                classIdPoint.x,
                maxClassScore * objectness,
                {
                    {}
                }
            )
        }
    }
}