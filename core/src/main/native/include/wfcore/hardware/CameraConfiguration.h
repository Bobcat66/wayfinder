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

#include "wfcore/video/video_types.h"
#include <opencv2/core.hpp>
#include <string>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <optional>

namespace wf {

    enum CameraBackend {
        CSCORE,
        REALSENSE, //WIP
        GSTREAMER, //WIP
        LIBCAMERA, //WIP
    };

    enum CamControl {
        EXPOSURE,
        BRIGHTNESS,
        ISO,
        SHUTTER,
        FOCUS,
        ZOOM,
        WHITE_BALANCE,
        SHARPNESS,
        SATURATION,
        CONTRAST,
        GAMMA,
        HUE
    };

    struct CameraIntrinsics {
        cv::Mat cameraMatrix;
        cv::Mat distCoeffs;
    };

    struct CameraConfiguration {
        std::string devpath;
        CameraBackend backend;
        StreamFormat format;
        std::map<cv::Size,CameraIntrinsics> intrinsics; // Didn't feel like making a hash function, plus this shouldn't be accessed in the hot path anyways
        std::unordered_map<CamControl,double> controls; 
    };

}