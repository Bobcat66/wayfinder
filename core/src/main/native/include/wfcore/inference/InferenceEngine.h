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

#include <vector>
#include <string>

#include <opencv2/core.hpp>
#include "wfcore/video/video_types.h"
#include "wfcore/inference/Tensorizer.h"
#include "wfcore/inference/ObjectDetection.h"
#include "wfcore/hardware/CameraConfiguration.h"
#include "wfcore/common/status/StatusfulObject.h"
#include <optional>
#include <array>

// TODO: Figure out resizing and letterboxing for Inference, also restructure Inference pipeline
// Object detection postprocessing should be done in the pipeline, not the Inference Engine
namespace wf {

    struct IEFilteringParams {
        float nmsThreshold;
        float confidenceThreshold;
    };

    enum class EngineStatus {
        Ok,
        ModelNotLoaded,
        InvalidModelFormat,
        ModelNotFound,
        Unknown,
        InvalidInput
    };

    class InferenceEngine : public StatusfulObject<EngineStatus,EngineStatus::Ok> {
    public:
        virtual ~InferenceEngine() = default;
        virtual bool setFilteringParameters(const IEFilteringParams& params) = 0;
        virtual bool setTensorParameters(const TensorParameters& params) = 0;
        virtual bool loadModel(const std::string& modelPath) = 0;
        virtual bool infer(const cv::Mat& data, const FrameMetadata& meta, std::vector<RawBbox>& output) noexcept = 0;
        virtual std::string modelFormat() const = 0; // the model file extension expected by this inference engine

        virtual const TensorParameters& getTensorParameters() { return tensorizer.getTensorParameters(); }
        virtual const IEFilteringParams& getFilteringParameters() { return filterParams; }
        virtual std::string getModelPath() { return modelPath; }
    protected:
        std::string modelPath;
        Tensorizer tensorizer;
        IEFilteringParams filterParams;
    };
}