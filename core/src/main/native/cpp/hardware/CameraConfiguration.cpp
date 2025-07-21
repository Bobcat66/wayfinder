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

#include "wfcore/hardware/CameraConfiguration.h"
#include "wfcore/common/logging.h"
#include <opencv2/core.hpp>
#include <unordered_map>

namespace impl {

    using namespace wf;
    cv::Mat createIntrinsicsMatrix(double fx, double fy, double cx, double cy) {
        return (cv::Mat_<double>(3, 3) <<
            fx,  0, cx,
            0, fy, cy,
            0,  0,  1);
    }

    static const std::unordered_map<CameraBackend,std::string> backendStringMap = {
        {CameraBackend::CSCORE,"CSCORE"},
        {CameraBackend::REALSENSE,"REALSENSE"},
        {CameraBackend::GSTREAMER,"GSTREAMER"},
        {CameraBackend::LIBCAMERA,"LIBCAMERA"}
    };

    static const std::unordered_map<std::string,CameraBackend> backendMap = {
        {"CSCORE",CameraBackend::CSCORE},
        {"REALSENSE",CameraBackend::REALSENSE},
        {"GSTREAMER",CameraBackend::GSTREAMER},
        {"LIBCAMERA",CameraBackend::LIBCAMERA}
    };

    static const std::unordered_map<CamControl,std::string> camControlStringMap = {
        {CamControl::EXPOSURE,"EXPOSURE"},
        {CamControl::AUTO_EXPOSURE,"AUTO_EXPOSURE"},
        {CamControl::BRIGHTNESS,"BRIGHTNESS"},
        {CamControl::ISO,"ISO"},
        {CamControl::SHUTTER,"SHUTTER"},
        {CamControl::FOCUS,"FOCUS"},
        {CamControl::ZOOM,"ZOOM"},
        {CamControl::WHITE_BALANCE,"WHITE_BALANCE"},
        {CamControl::AUTO_WHITE_BALANCE,"AUTO_WHITE_BALANCE"},
        {CamControl::SHARPNESS,"SHARPNESS"},
        {CamControl::SATURATION,"SATURATION"},
        {CamControl::CONTRAST,"CONTRAST"},
        {CamControl::GAMMA,"GAMMA"},
        {CamControl::HUE,"HUE"}
    };

    static const std::unordered_map<std::string,CamControl> camControlMap = {
        {"EXPOSURE",CamControl::EXPOSURE},
        {"AUTO_EXPOSURE",CamControl::AUTO_EXPOSURE},
        {"BRIGHTNESS",CamControl::BRIGHTNESS},
        {"ISO",CamControl::ISO},
        {"SHUTTER",CamControl::SHUTTER},
        {"FOCUS",CamControl::FOCUS},
        {"ZOOM",CamControl::ZOOM},
        {"WHITE_BALANCE",CamControl::WHITE_BALANCE},
        {"AUTO_WHITE_BALANCE",CamControl::AUTO_WHITE_BALANCE},
        {"SHARPNESS",CamControl::SHARPNESS},
        {"SATURATION",CamControl::SATURATION},
        {"CONTRAST",CamControl::CONTRAST},
        {"GAMMA",CamControl::GAMMA},
        {"HUE",CamControl::HUE}
    };
}

namespace wf {
    using enum JSONStatus;

    JSONStatusResult<JSONObject> CameraIntrinsics::toJSON_impl(const CameraIntrinsics& object) {

        // cameraMatrix verification
        if (!(object.cameraMatrix.rows == 3 
            && object.cameraMatrix.cols == 3 
            && object.cameraMatrix.type() == CV_64FC1)) {
            jsonLogger()->error("CameraIntrinsics.cameraMatrix is invalid");
            return JSONStatusResult<JSONObject>::failure(SchemaViolation);
        }
        // distCoeffs verification
        if (!(object.distCoeffs.rows == 1 
            && object.distCoeffs.cols <= 8
            && object.distCoeffs.cols >= 5
            && object.distCoeffs.type() == CV_64FC1)) {
            jsonLogger()->error("CameraIntrinsics.distCoeffs is invalid");
            return JSONStatusResult<JSONObject>::failure(SchemaViolation);
        }


        // Construct JSON object
        std::vector<double> distCoeffsVec;
        distCoeffsVec.assign(object.distCoeffs.ptr<double>(0),object.distCoeffs.ptr<double>(0) + object.distCoeffs.cols);
        try {
            JSONObject j = {
                {"resolution",{
                    {"width", object.resolution.width},
                    {"height",object.resolution.height}
                }},
                {"matrix",{
                    {"fx",object.cameraMatrix.at<double>(0,0)},
                    {"fy",object.cameraMatrix.at<double>(1,1)},
                    {"cx",object.cameraMatrix.at<double>(0,2)},
                    {"cy",object.cameraMatrix.at<double>(1,2)}
                }},
                {"distortion",std::move(distCoeffsVec)}
            };
            return JSONStatusResult<JSONObject>::success(std::move(j));
        } catch (const nlohmann::json::exception& e) {
            jsonLogger()->error("JSON error while serializing CameraIntrinsics: {}",e.what());
            return JSONStatusResult<JSONObject>::failure(Unknown);
        }
    }

    JSONStatusResult<CameraIntrinsics> CameraIntrinsics::fromJSON_impl(const JSONObject& jobject) {
        // Verifying fields
        if (!jobject.is_object()) {
            jsonLogger()->error("camera_intrinsics is not an object");
            return JSONStatusResult<CameraIntrinsics>::failure(InvalidType);
        }
        if (!validateProperties(jobject,{"resolution","matrix","distortion"},"camera_intrinsics")) {
            return JSONStatusResult<CameraIntrinsics>::failure(PropertyNotFound);
        }

        // Verifying resolution
        if (!jobject["resolution"].is_object()) {
            jsonLogger()->error("camera_intrinsics.resolution is not an object");
            return JSONStatusResult<CameraIntrinsics>::failure(InvalidType);
        }
        if (!validateProperties(jobject["resolution"],{"width","height"})) {
            return JSONStatusResult<CameraIntrinsics>::failure(PropertyNotFound);
        }

        // Verifying cameraMatrix
        if (!jobject["matrix"].is_object()) {
            jsonLogger()->error("camera_intrinsics.matrix is not an object");
            return JSONStatusResult<CameraIntrinsics>::failure(InvalidType);
        }
        if (!validateProperties(jobject["matrix"],{"fx","fy","cx","cy"},"camera_intrinsics.matrix")) {
            return JSONStatusResult<CameraIntrinsics>::failure(PropertyNotFound);
        }

        // Verifying distCoeffs
        if (!jobject["distortion"].is_array()) {
            jsonLogger()->error("camera_intrinsics.matrix is not an array");
            return JSONStatusResult<CameraIntrinsics>::failure(InvalidType);
        }

        // Construct object
        double width,height,fx,fy,cx,cy;
        std::vector<double> distVec;

        // resolution
        try {
            width = jobject["resolution"]["width"].get<double>();
            height = jobject["resolution"]["height"].get<double>();
        } catch (const nlohmann::json::type_error& e) {
            jsonLogger()->error("Type error while parsing camera_intrinsics.resolution: {}",e.what());
            return JSONStatusResult<CameraIntrinsics>::failure(InvalidType);
        } catch (const nlohmann::json::exception& e) {
            jsonLogger()->error("JSON error while parsing camera_intrinsics.resolution: {}",e.what());
            return JSONStatusResult<CameraIntrinsics>::failure(Unknown);
        }

        // matrix
        try {
            fx = jobject["matrix"]["fx"].get<double>();
            fy = jobject["matrix"]["fy"].get<double>();
            cx = jobject["matrix"]["cx"].get<double>();
            cy = jobject["matrix"]["cy"].get<double>();
        } catch (const nlohmann::json::type_error& e) {
            jsonLogger()->error("Type error while parsing camera_intrinsics.matrix: {}",e.what());
            return JSONStatusResult<CameraIntrinsics>::failure(InvalidType);
        } catch (const nlohmann::json::exception& e) {
            jsonLogger()->error("JSON error while parsing camera_intrinsics.matrix: {}",e.what());
            return JSONStatusResult<CameraIntrinsics>::failure(Unknown);
        }

        // distortion
        try {
            distVec = jobject["distortion"].get<std::vector<double>>();
        } catch (const nlohmann::json::type_error& e) {
            jsonLogger()->error("Type error while parsing camera_intrinsics.distortion: {}",e.what());
            return JSONStatusResult<CameraIntrinsics>::failure(InvalidType);
        } catch (const nlohmann::json::exception& e) {
            jsonLogger()->error("JSON error while parsing camera_intrinsics.distortion: {}",e.what());
            return JSONStatusResult<CameraIntrinsics>::failure(Unknown);
        }

        if (!(distVec.size() >= 5 && distVec.size() <= 8)){
            jsonLogger()->error("camera_intrinsics.distortion is the wrong size");
            return JSONStatusResult<CameraIntrinsics>::failure(InvalidType);
        }

        cv::Mat distCoeffs = cv::Mat(distVec).reshape(1,1).clone();
        auto matrix = impl::createIntrinsicsMatrix(fx,fy,cx,cy);
        cv::Size resolution(width,height);
        
        return JSONStatusResult<CameraIntrinsics>::success(std::in_place,resolution,matrix,distCoeffs);
    }


    JSONStatusResult<JSONObject> CameraConfiguration::toJSON_impl(const CameraConfiguration& object) {
        std::string backendStr = 
    }

    JSONStatusResult<CameraConfiguration> CameraConfiguration::fromJSON_impl(const JSONObject& jobject) {
        return JSONStatusResult<CameraConfiguration>::failure(Unknown); // Placeholder
    }
}