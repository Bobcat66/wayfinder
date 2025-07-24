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
#include "wfcore/common/status.h"
#include "wfcore/common/wfexcept.h"
#include "wfcore/common/json_utils.h"
#include <opencv2/core.hpp>
#include <string>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <optional>

namespace wf {

    enum class CameraBackend {
        CSCORE,
        REALSENSE, //WIP
        GSTREAMER, //WIP
        LIBCAMERA, //WIP
        UNKNOWN
    };

    // TODO: Enumerate more controls. That is for after 1.0, though
    enum class CamControl {
        EXPOSURE,
        AUTO_EXPOSURE,
        BRIGHTNESS,
        ISO,
        SHUTTER,
        FOCUS,
        ZOOM,
        WHITE_BALANCE,
        AUTO_WHITE_BALANCE,
        SHARPNESS,
        SATURATION,
        CONTRAST,
        GAMMA,
        HUE,
        UNKNOWN
    };

    struct CameraIntrinsics : public JSONSerializable<CameraIntrinsics> {
        cv::Size resolution;
        cv::Mat cameraMatrix;
        cv::Mat distCoeffs;

        CameraIntrinsics(cv::Size resolution_, cv::Mat cameraMatrix_, cv::Mat distCoeffs_)
        : resolution(std::move(resolution_)), cameraMatrix(std::move(cameraMatrix_)), distCoeffs(std::move(distCoeffs_)) {}

        static WFResult<JSON> toJSON_impl(const CameraIntrinsics& object);
        static WFResult<CameraIntrinsics> fromJSON_impl(const JSON& jobject);
        static const JSONValidationFunctor* getValidator_impl();
    };

    struct CameraConfiguration : public JSONSerializable<CameraConfiguration> {

        CameraConfiguration(
            std::string devpath_,
            CameraBackend backend_,StreamFormat format_,
            std::unordered_map<CamControl,std::string> controlAliases_,
            std::vector<CameraIntrinsics> calibrations_,
            std::unordered_map<CamControl,int> controls_
        ) : devpath(std::move(devpath_)), backend(backend_), format(std::move(format_)),
        controlAliases(std::move(controlAliases_)), calibrations(std::move(calibrations_)),
        controls(std::move(controls_)) {}
        std::string devpath;
        CameraBackend backend;
        StreamFormat format;
        std::unordered_map<CamControl,std::string> controlAliases; // Aliases for camera controls, for V4L2/CScore interop
        std::vector<CameraIntrinsics> calibrations;
        std::unordered_map<CamControl,int> controls; 

        static WFResult<JSON> toJSON_impl(const CameraConfiguration& object);
        static WFResult<CameraConfiguration> fromJSON_impl(const JSON& jobject);
        static const JSONValidationFunctor* getValidator_impl();
    };

}