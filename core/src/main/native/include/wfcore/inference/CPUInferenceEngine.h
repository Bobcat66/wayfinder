/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of Wayfinder.
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

#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>

// A simple reference implementation of a CPU inference engine, using OpenCV's DNN module.
namespace wf {
    class CPUInferenceEngine : public InferenceEngine {
    public:
        CPUInferenceEngine();
        std::string modelFormat() const override {return "onnx";}
        bool loadModel(const std::string& modelPath) override;
        [[nodiscard]] 
        std::vector<ObjectDetection> infer(const Frame& input) noexcept override;
    private:

    }
}