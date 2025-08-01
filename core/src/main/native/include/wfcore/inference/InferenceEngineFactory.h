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

#include "wfcore/common/status.h"
#include "wfcore/configuration/ResourceManager.h"
#include "wfcore/pipeline/ObjectDetectionPipelineConfiguration.h"
#include "wfcore/inference/InferenceEngine.h"
#include "unordered_map"
#include <memory>
#include <functional>
#include <filesystem>

namespace wf {

    class InferenceEngineFactory {
    public:
        InferenceEngineFactory(const ResourceManager& resourceManager_): resourceManager(resourceManager_) {}
        WFResult<std::unique_ptr<InferenceEngine>> makeInferenceEngine(
            ModelArch modelArch,
            InferenceEngineType engineType,
            std::string modelFile,
            TensorParameters tensorParams,
            IEFilteringParams filterParams
        );
        WFResult<std::unique_ptr<InferenceEngine>> makeInferenceEngine(const ObjectDetectionPipelineConfiguration& config) {
            return makeInferenceEngine(
                config.modelArch,
                config.engineType,
                config.modelFile,
                config.tensorParams,
                config.filterParams
            );
        }
    private:
        const ResourceManager& resourceManager;
    };
}