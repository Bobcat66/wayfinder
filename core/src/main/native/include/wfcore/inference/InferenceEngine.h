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

#include <vector>
#include <string>

#include <opencv2/core.hpp>
#include "wfcore/video/video_types.h"

namespace wf {
    struct ObjectDetection {
        int objectClass;
        double confidence;
        double percentArea;
        std::vector<cv::Point2d> cornerPixels;
        std::vector<cv::Point2d> cornerAngles;
        ObjectDetection(
            int objectClass_, double confidence_, double percentArea_,
            std::vector<cv::Point2d> cornerPixels_,
            std::vector<cv::Point2d> cornerAngles_
        ) : objectClass(objectClass_), confidence(confidence_), percentArea(percentArea_),
            cornerPixels(std::move(cornerPixels_)), cornerAngles(std::move(cornerAngles_)) {}
    };

    class InferenceEngine {
    public:
        virtual ~InferenceEngine() = default;
        virtual bool loadModel(const std::string& modelPath) = 0;
        [[nodiscard]] 
        virtual std::vector<ObjectDetection> infer(const Frame& input) noexcept = 0;
        virtual std::string modelFormat() const = 0; // the model file extension expected by this inference engine
    };
}