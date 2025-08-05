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
#include <functional>
#include <memory>
#include <filesystem>
// TODO: Figure out resizing and letterboxing for Inference, also restructure Inference pipeline
// Object detection postprocessing should be done in the pipeline, not the Inference Engine
namespace wf {

    struct IEFilteringParams {
        float nmsThreshold = 0.0;
        float confidenceThreshold = 0.0;
    };

    // Each
    class InferenceEngine {
    public:
        virtual ~InferenceEngine() = default;
        virtual void setFilteringParameters(IEFilteringParams params) = 0;
        virtual void setTensorParameters(TensorParameters params) = 0;
        virtual WFStatusResult infer(const cv::Mat& data, const FrameMetadata& meta, std::vector<RawBbox>& output) noexcept = 0;
        virtual const std::string& modelFormat() const = 0; // the model file extension expected by this inference engine

        virtual const TensorParameters& getTensorParameters() { return tensorizer.getTensorParameters(); }
        virtual const IEFilteringParams& getFilteringParameters() { return filterParams; }
        virtual const std::filesystem::path getModelPath() { return modelPath; }
        virtual InferenceEngineType getEngineType() const noexcept = 0;
        virtual ModelArch getModelArch() const noexcept = 0;
    protected:
        std::filesystem::path modelPath;
        Tensorizer tensorizer;
        IEFilteringParams filterParams;
    };
    
}