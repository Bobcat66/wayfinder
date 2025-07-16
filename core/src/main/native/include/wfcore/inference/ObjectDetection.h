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

#include <array>
#include <opencv2/core.hpp>

namespace wf {

    struct RawBbox {
        float x; // x coordinate of the top-left corner
        float y; // y coordinate of the top-left corner
        float width; 
        float height;
        int objectClass;
        float confidence;
    };

    struct ObjectDetection {
        int objectClass;
        float confidence;
        float percentArea;
        std::array<cv::Point2f, 4> cornerPixels;
        std::array<cv::Point2f, 4> cornerAngles;
        ObjectDetection(
            int objectClass_, float confidence_, float percentArea_,
            std::array<cv::Point2f, 4> cornerPixels_,
            std::array<cv::Point2f, 4> cornerAngles_
        ) : objectClass(objectClass_), confidence(confidence_), percentArea(percentArea_),
            cornerPixels(std::move(cornerPixels_)), cornerAngles(std::move(cornerAngles_)) {}
    };
    
}