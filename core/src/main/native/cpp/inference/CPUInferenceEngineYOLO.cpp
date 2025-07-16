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
#include "wfcore/inference/postproc_utils.h"
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
    }
    [[nodiscard]] 
    std::vector<RawBbox> CPUInferenceEngineYOLO::infer(const cv::Mat& data, const FrameMetadata& meta) noexcept {
        this->tensorizer.tensorize(data, reinterpret_cast<float*>(blob.data));
        model.setInput(blob);
        cv::Mat output = model.forward();
        // Output Shape: [1, number of detections, 5 + number of classes]
        int numDetections = output.rows;
        std::vector<RawBbox> detections;
        detections.reserve(numDetections);
        int num_classes = output.cols - 5;
        objclass_buffer.clear();
        index_buffer.clear();
        bboxd_buffer.clear();
        confidence_buffer.clear();
        for (int i = 0; i < output.rows; ++i) {
            // Detection format: [x center, y center, width, height, objectness, class_confidences...]
            float* data = output.ptr<float>(i);
            int objectClass = getClassYOLO(data,num_classes); // Returns the class with the highest confidence score
            float confidence = getConfidenceYOLO(data,objectClass); // Class confidence score * objectness
            float width = data[2];
            float height = data[3];
            float topleft_x = data[0] - (width/2.0f);
            float topleft_y = data[1] - (height/2.0f);
            bboxd_buffer.emplace_back(
                static_cast<double>(topleft_x),
                static_cast<double>(topleft_y),
                static_cast<double>(width),
                static_cast<double>(height)
            );
            confidence_buffer.push_back(confidence);
            objclass_buffer.push_back(objectClass);
        }
        cv::dnn::NMSBoxes(
            bboxd_buffer,
            confidence_buffer,
            this->filterParams.confidenceThreshold,
            this->filterParams.nmsThreshold,
            index_buffer
        );
        for (int index : index_buffer) {
            auto bboxd = bboxd_buffer[index];
            detections.emplace_back(
                static_cast<float>(bboxd.x),
                static_cast<float>(bboxd.y),
                static_cast<float>(bboxd.width),
                static_cast<float>(bboxd.height),
                objclass_buffer[index],
                confidence_buffer[index]
            );
        }
        return detections;
    }
}