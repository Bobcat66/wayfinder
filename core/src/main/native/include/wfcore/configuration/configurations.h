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

#include <opencv2/core.hpp>
#include <unordered_set>
#include <string>

namespace wf {


    struct ObjectDetectPipelineConfiguration {
        std::string model;
        double confidence_threshold;
    };

    struct CameraIntrinsics {
        cv::Mat cameraMatrix;
        cv::Mat distCoeffs;
    };

    struct CameraConfiguration {
        int id;
        CameraIntrinsics intrinsics; 
    };
}