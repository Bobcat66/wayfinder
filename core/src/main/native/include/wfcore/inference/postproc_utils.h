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

#include <opencv2/opencv.hpp>
#include <vector>
#include <array>
#include "wfcore/inference/ObjectDetection.h"

namespace wf {
    std::array<cv::Point2f, 4> getCorners_CVRect2d(const cv::Rect2d& bbox);
    std::array<cv::Point2f, 4> getCorners_CVRect2f(const cv::Rect2f& bbox);
    std::array<cv::Point2f, 4> getCorners_RawBbox(const RawBbox& bbox);
    double getConfidenceYOLO(float* yolo_result, int obj_class);
    int getClassYOLO(float* yolo_result, int num_classes);
}