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

// This class is meant as a stand-in for inference engine types that aren't implemented for whatever reason
#include "wfcore/inference/InferenceEngine.h"
#include "wfcore/common/status.h"
#include <memory>

namespace wf {
    class BrokenInferenceEngine : public InferenceEngine {
    public:
        const std::string& modelFormat() const override {
            static const std::string format("N/A");
            return format;
        }
        void setFilteringParameters(IEFilteringParams params) override {}
        void setTensorParameters(TensorParameters params) override {};
        WFStatusResult infer(const cv::Mat& data, const FrameMetadata& meta, std::vector<RawBbox>& output) noexcept override {
            return WFStatusResult::failure(WFStatus::NOT_IMPLEMENTED);
        }
        static WFResult<std::unique_ptr<BrokenInferenceEngine>> creator_impl(
            std::filesystem::path modelPath,
            TensorParameters tensorParams,
            IEFilteringParams filterParams
        ) {
            return WFResult<std::unique_ptr<BrokenInferenceEngine>>::failure(WFStatus::NOT_IMPLEMENTED);
        }
        InferenceEngineType getEngineType() const noexcept override { return InferenceEngineType::Unknown; }
        ModelArch getModelArch() const noexcept override { return ModelArch::Unknown; }
    };
}