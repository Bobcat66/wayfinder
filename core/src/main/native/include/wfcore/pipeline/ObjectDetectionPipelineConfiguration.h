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

#include "wfcore/inference/InferenceEngine.h"
#include "wfcore/video/video_types.h"
#include "wfcore/inference/Tensorizer.h"
#include "wfcore/common/json_utils.h"
#include <string>

namespace wf {

    struct ObjectDetectionPipelineConfiguration : JSONSerializable<ObjectDetectionPipelineConfiguration> {
        std::string modelFile; // Path to the model file, relative to the resources/models subdir
        ModelArch modelArch; // Architecture of the model
        InferenceEngineType engineType; // Type of inference engine to use
        TensorParameters tensorParams; // Parameters for the tensorizer. The resolution and format of images input to the model should match the tensor parameters, NOT the native resolution
        ImageEncoding modelColorSpace; // Color space the model expects pixels to be in
        IEFilteringParams filterParams;

        ObjectDetectionPipelineConfiguration(
            std::string modelFile_,
            ModelArch modelArch_,
            InferenceEngineType engineType_,
            TensorParameters tensorParams_,
            ImageEncoding modelColorSpace_,
            IEFilteringParams filterParams_
        )
        : modelFile(std::move(modelFile_))
        , modelArch(modelArch_)
        , engineType(engineType_)
        , tensorParams(std::move(tensorParams_))
        , modelColorSpace(modelColorSpace_)
        , filterParams(std::move(filterParams_)) {}

        static const JSONValidationFunctor* getValidator_impl();
        static WFResult<JSON> toJSON_impl(const ObjectDetectionPipelineConfiguration& object);
        static WFResult<ObjectDetectionPipelineConfiguration> fromJSON_impl(const JSON& jobject);

    };
}