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
#include "wfcore/inference/inference_utils.h"
#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>
#include <cassert>
#include <opencv2/calib3d.hpp>
#include <cmath>

namespace wf {
    CPUInferenceEngineYOLO::CPUInferenceEngineYOLO(){
        corners_buffer.reserve(4);
        norm_corners_buffer.reserve(4);
    }
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
        int num_classes = output.cols - 5;
        for (int i = 0; i < output.rows; ++i) {
            corners_buffer.clear();
            norm_corners_buffer.clear();
            float* data = output.ptr<float>(i);
            double width = data[2];
            double height = data[3];
            auto corners = getCornersYOLO(data);
            corners_buffer.assign(corners.begin(),corners.end());
            cv::undistortPoints(
                corners_buffer,
                norm_corners_buffer,
                intrinsics.cameraMatrix,
                intrinsics.distCoeffs
            );
            int objectClass = getClassYOLO(data,num_classes);
            double confidence = getConfidenceYOLO(data,objectClass);
            detections.emplace_back(
                objectClass,
                confidence,
                (width * height) / (input.format.rows * input.format.cols),
                std::array<cv::Point2d, 4>{
                    corners_buffer[0],
                    corners_buffer[1],
                    corners_buffer[2],
                    corners_buffer[3]
                },
                std::array<cv::Point2d, 4>{
                    cv::Point2d{std::atan(norm_corners_buffer[0].x),std::atan(norm_corners_buffer[0].y)},
                    cv::Point2d{std::atan(norm_corners_buffer[1].x),std::atan(norm_corners_buffer[1].y)},
                    cv::Point2d{std::atan(norm_corners_buffer[2].x),std::atan(norm_corners_buffer[2].y)},
                    cv::Point2d{std::atan(norm_corners_buffer[3].x),std::atan(norm_corners_buffer[3].y)},
                }
            );
        }
        return detections;
    }
}