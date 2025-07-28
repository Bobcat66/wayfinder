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
#include "wfcore/common/wfassert.h"
#include "wfcore/common/wfexcept.h"
#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>
#include <cassert>
#include <opencv2/calib3d.hpp>
#include <cmath>

// TODO: Make this use the engine status codes and messages
namespace wf {

    using enum WFStatus;
    
    CPUInferenceEngineYOLO::CPUInferenceEngineYOLO(std::filesystem::path modelPath, TensorParameters tensorParams, IEFilteringParams filterParams) {
        setTensorParameters(std::move(tensorParams));
        setFilteringParameters(std::move(filterParams));
        try {
            model = cv::dnn::readNetFromONNX(modelPath);
        } catch (const cv::Exception& e) {
            throw bad_model("OpenCV Error while loading model: {}", e.what());
        }
        model.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        model.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        this->modelPath = modelPath;
        corners_buffer.reserve(4);
        norm_corners_buffer.reserve(4);
    }

    WFResult<std::unique_ptr<CPUInferenceEngineYOLO>> CPUInferenceEngineYOLO::creator_impl(
        std::filesystem::path modelPath,
        TensorParameters tensorParams,
        IEFilteringParams filterParams
    ) {
        try {
            return WFResult<std::unique_ptr<CPUInferenceEngineYOLO>>::success(
                std::move(std::make_unique<CPUInferenceEngineYOLO>(
                    std::move(modelPath),
                    std::move(tensorParams),
                    std::move(filterParams)
                ))
            );
        } catch (const wfexception& e) {
            return WFResult<std::unique_ptr<CPUInferenceEngineYOLO>>::failure(
                e.status(),
                e.what()
            );
        }
    }

    void CPUInferenceEngineYOLO::setTensorParameters(TensorParameters params) {
        this->tensorizer.setTensorParameters(params);
        int dims[] = {
            1,
            params.interleaved ? params.height : params.channels,
            params.interleaved ? params.width : params.height,
            params.interleaved ? params.channels : params.width
        };
        blob = cv::Mat(4, dims, CV_32F);
        WF_Assert(blob.isContinuous());
    }
    
    WFStatusResult CPUInferenceEngineYOLO::infer(const cv::Mat& data, const FrameMetadata& meta, std::vector<RawBbox>& output) noexcept {
        output.clear();
        cv::Mat rawOutput;
        this->tensorizer.tensorize(data, reinterpret_cast<float*>(blob.data));
        try {
            model.setInput(blob);
        } catch (const cv::Exception& e) {
            return WFStatusResult::failure(
                INFERENCE_BAD_INPUT,
                "OpenCV Error while inputting blob: {}", e.what()
            );
        }
        try {
            rawOutput = model.forward();
        } catch (const cv::Exception& e) {
            return WFStatusResult::failure(
                INFERENCE_BAD_PASS,
                "OpenCV Error while running forward pass: {}", e.what()
            );
        }
        // Output Shape: [1, number of detections, 5 + number of classes]
        int numDetections = rawOutput.rows;
        output.reserve(numDetections);
        int num_classes = rawOutput.cols - 5;
        objclass_buffer.clear();
        index_buffer.clear();
        bboxd_buffer.clear();
        confidence_buffer.clear();
        for (int i = 0; i < rawOutput.rows; ++i) {
            // Detection format: [x center, y center, width, height, objectness, class_confidences...]
            float* data = rawOutput.ptr<float>(i);
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
            output.emplace_back(
                bboxd.x,
                bboxd.y,
                bboxd.width,
                bboxd.height,
                objclass_buffer[index],
                confidence_buffer[index]
            );
        }
        return WFStatusResult::success();
    }
}