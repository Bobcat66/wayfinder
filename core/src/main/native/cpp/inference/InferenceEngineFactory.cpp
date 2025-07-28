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

#include "wfcore/inference/InferenceEngineFactory.h"
#include "wfcore/inference/CPUInferenceEngineYOLO.h"
#include "wfcore/inference/BrokenInferenceEngine.h"
#include "wfcore/inference/InferenceEngineCreator.h"
#include <unordered_map>
#include <functional>

namespace impl {
    using namespace wf;
    using InferenceEngineCreatorCreator = std::function<InferenceEngineCreator(void)>;

    static const std::unordered_map<ModelArch,std::unordered_map<InferenceEngineType,InferenceEngineCreatorCreator>> creatorMap = {
        {YOLO, {
            CV_CPU, InferenceEngineCreator::getCreator<CPUInferenceEngineYOLO>
        }}
    };

    static InferenceEngineCreator getInferenceEngineCreator(ModelArch modelArch, InferenceEngineType engineType) {
        auto archit = creatorMap.find(modelArch);
        if (archit == creatorMap.end()) {
            // Not implemented
            return InferenceEngineCreator::getCreator<BrokenInferenceEngine>();
        }
        auto etypeit = archit->second.find(engineType);
        if (etypeit == archit->second.end()) {
            // Not implemented
            return InferenceEngineCreator::getCreator<BrokenInferenceEngine>();
        }
        return (etypeit->second)();
    }
}

namespace wf {
    namespace fs = std::filesystem;
    WFResult<std::unique_ptr<InferenceEngine>> InferenceEngineFactory::makeInferenceEngine(
        ModelArch modelArch,
        InferenceEngineType engineType,
        std::string modelFile,
        TensorParameters tensorParams,
        IEFilteringParams filterParams
    ) {
        auto creator = impl::getInferenceEngineCreator(modelArch,engineType);
        fs::path modelPath;
        if (auto pathres = resourceManager.resolveLocalFile("models",modelFile)) {
            modelPath = std::move(pathres.value());
        } else {
            return WFResult<std::unique_ptr<InferenceEngine>>::propagateFail(pathres);
        }
        return creator(std::move(modelPath),std::move(tensorParams),std::move(filterParams));
    }
}