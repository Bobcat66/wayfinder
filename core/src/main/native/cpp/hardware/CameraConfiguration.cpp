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
#include <format>

// TODO: refactor to use getProperty();
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
    using enum WFStatus;

    WFResult<JSON> CameraIntrinsics::toJSON_impl(const CameraIntrinsics& object) {

        // cameraMatrix verification
        if (!(object.cameraMatrix.rows == 3 
            && object.cameraMatrix.cols == 3 
            && object.cameraMatrix.type() == CV_64FC1)) {
            jsonLogger()->error("CameraIntrinsics.cameraMatrix is invalid");
            return WFResult<JSON>::failure(JSON_SCHEMA_VIOLATION);
        }
        // distCoeffs verification
        if (!(object.distCoeffs.rows == 1 
            && object.distCoeffs.cols <= 8
            && object.distCoeffs.cols >= 5
            && object.distCoeffs.type() == CV_64FC1)) {
            jsonLogger()->error("CameraIntrinsics.distCoeffs is invalid");
            return WFResult<JSON>::failure(JSON_SCHEMA_VIOLATION);
        }


        // Construct JSON object
        std::vector<double> distCoeffsVec;
        distCoeffsVec.assign(object.distCoeffs.ptr<double>(0),object.distCoeffs.ptr<double>(0) + object.distCoeffs.cols);
        try {
            JSON j = {
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
            return WFResult<JSON>::success(std::move(j));
        } catch (const nlohmann::json::exception& e) {
            jsonLogger()->error("JSON error while serializing CameraIntrinsics: {}",e.what());
            return WFResult<JSON>::failure(JSON_UNKNOWN);
        }
    }

    WFResult<CameraIntrinsics> CameraIntrinsics::fromJSON_impl(const JSON& jobject) {
        // Verifying fields
        if (!jobject.is_object()) {
            jsonLogger()->error("camera_intrinsics is not an object");
            return WFResult<CameraIntrinsics>::failure(JSON_INVALID_TYPE);
        }
        if (!validateProperties(jobject,{"resolution","matrix","distortion"},"camera_intrinsics")) {
            return WFResult<CameraIntrinsics>::failure(JSON_PROPERTY_NOT_FOUND);
        }

        // Verifying resolution
        if (!jobject["resolution"].is_object()) {
            jsonLogger()->error("camera_intrinsics.resolution is not an object");
            return WFResult<CameraIntrinsics>::failure(JSON_INVALID_TYPE);
        }
        if (!validateProperties(jobject["resolution"],{"width","height"})) {
            return WFResult<CameraIntrinsics>::failure(JSON_PROPERTY_NOT_FOUND);
        }

        // Verifying cameraMatrix
        if (!jobject["matrix"].is_object()) {
            jsonLogger()->error("camera_intrinsics.matrix is not an object");
            return WFResult<CameraIntrinsics>::failure(JSON_INVALID_TYPE);
        }
        if (!validateProperties(jobject["matrix"],{"fx","fy","cx","cy"},"camera_intrinsics.matrix")) {
            return WFResult<CameraIntrinsics>::failure(JSON_PROPERTY_NOT_FOUND);
        }

        // Verifying distCoeffs
        if (!jobject["distortion"].is_array()) {
            jsonLogger()->error("camera_intrinsics.matrix is not an array");
            return WFResult<CameraIntrinsics>::failure(JSON_INVALID_TYPE);
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
            return WFResult<CameraIntrinsics>::failure(JSON_INVALID_TYPE);
        } catch (const nlohmann::json::exception& e) {
            jsonLogger()->error("JSON error while parsing camera_intrinsics.resolution: {}",e.what());
            return WFResult<CameraIntrinsics>::failure(JSON_UNKNOWN);
        }

        // matrix
        try {
            fx = jobject["matrix"]["fx"].get<double>();
            fy = jobject["matrix"]["fy"].get<double>();
            cx = jobject["matrix"]["cx"].get<double>();
            cy = jobject["matrix"]["cy"].get<double>();
        } catch (const nlohmann::json::type_error& e) {
            jsonLogger()->error("Type error while parsing camera_intrinsics.matrix: {}",e.what());
            return WFResult<CameraIntrinsics>::failure(JSON_INVALID_TYPE);
        } catch (const nlohmann::json::exception& e) {
            jsonLogger()->error("JSON error while parsing camera_intrinsics.matrix: {}",e.what());
            return WFResult<CameraIntrinsics>::failure(JSON_UNKNOWN);
        }

        // distortion
        try {
            distVec = jobject["distortion"].get<std::vector<double>>();
        } catch (const nlohmann::json::type_error& e) {
            jsonLogger()->error("Type error while parsing camera_intrinsics.distortion: {}",e.what());
            return WFResult<CameraIntrinsics>::failure(JSON_INVALID_TYPE);
        } catch (const nlohmann::json::exception& e) {
            jsonLogger()->error("JSON error while parsing camera_intrinsics.distortion: {}",e.what());
            return WFResult<CameraIntrinsics>::failure(JSON_UNKNOWN);
        }

        if (!(distVec.size() >= 5 && distVec.size() <= 8)){
            jsonLogger()->error("camera_intrinsics.distortion is the wrong size");
            return WFResult<CameraIntrinsics>::failure(JSON_INVALID_TYPE);
        }

        cv::Mat distCoeffs = cv::Mat(distVec).reshape(1,1).clone();
        auto matrix = impl::createIntrinsicsMatrix(fx,fy,cx,cy);
        cv::Size resolution(width,height);
        
        return WFResult<CameraIntrinsics>::success(
            std::in_place,
            resolution,
            std::move(matrix),
            std::move(distCoeffs)
        );
    }


    WFResult<JSON> CameraConfiguration::toJSON_impl(const CameraConfiguration& object) {
        auto backendIt = impl::backendStringMap.find(object.backend);
        if (backendIt == impl::backendStringMap.end()) [[ unlikely ]] {
            jsonLogger()->error("CameraConfiguration.backend is unrecognized");
            return WFResult<JSON>::failure(JSON_SCHEMA_VIOLATION);
        }
        std::string backendStr = backendIt->second;

        JSON format_jobject = JSON::object();
        if (auto jresult = StreamFormat::toJSON(object.format)) {
            format_jobject = std::move(jresult.value());
        } else {
            jsonLogger()->error("Error while parsing CameraConfiguration.format");
            return WFResult<JSON>::failure(jresult.status());
        }

        JSON controlAliases_jobject = JSON::object();
        for (const auto& [control,alias] : object.controlAliases) {
            auto controlIt = impl::camControlStringMap.find(control);
            if (controlIt == impl::camControlStringMap.end()) {
                jsonLogger()->error("CameraConfiguration.controlAliases contains an unrecognized control");
                return WFResult<JSON>::failure(JSON_SCHEMA_VIOLATION);
            }
            controlAliases_jobject[controlIt->second] = alias;
        }

        JSON calibrations_jobject = JSON::array();
        for (const auto& calibration : object.calibrations) {
            if (auto jresult = CameraIntrinsics::toJSON(calibration)) {
                calibrations_jobject.push_back(std::move(jresult.value()));
            } else {
                jsonLogger()->error("Error parsing CameraConfiguration.calibrations");
                return WFResult<JSON>::failure(jresult.status());
            }
        }

        JSON controls_jobject = JSON::object();
        for (const auto& [control,value] : object.controls) {
            auto controlIt = impl::camControlStringMap.find(control);
            if (controlIt == impl::camControlStringMap.end()) {
                jsonLogger()->error("CameraConfiguration.controls contains an unrecognized control");
                return WFResult<JSON>::failure(JSON_SCHEMA_VIOLATION);
            }
            controls_jobject[controlIt->second] = value;
        }

        try {
            JSON jobject = {
                {"devpath",object.devpath},
                {"backend",std::move(backendStr)},
                {"format",std::move(format_jobject)},
                {"controlAliases",std::move(controlAliases_jobject)},
                {"calibrations",std::move(calibrations_jobject)},
                {"controls",std::move(controls_jobject)}
            };
            return WFResult<JSON>::success(std::move(jobject));
        } catch (const nlohmann::json::exception& e) {
            jsonLogger()->error("JSON error while serializing CameraConfiguration: {}",e.what());
            return WFResult<JSON>::failure(JSON_UNKNOWN);
        }
    }

    WFResult<CameraConfiguration> CameraConfiguration::fromJSON_impl(const JSON& jobject) {

        // Object verification
        if (!jobject.is_object()) {
            jsonLogger()->error("camera_configuration is not an object");
            return WFResult<CameraConfiguration>::failure(JSON_INVALID_TYPE);
        }
        if (!validateProperties(jobject,{"devpath","backend","format","controlAliases"},"camera_configuration")) {
            return WFResult<CameraConfiguration>::failure(JSON_PROPERTY_NOT_FOUND);
        }

        std::string devpath;
        CameraBackend backend;

        // Devpath
        if (auto devpathRes = getProperty<std::string>(jobject,"devpath","camera_configuration")) {
            devpath = std::move(devpathRes.value());
        } else {
            return WFResult<CameraConfiguration>::failure(devpathRes.status());
        }

        // camera backend
        if (auto backendRes = getProperty<std::string>(jobject,"backend","camera_configuration")) {
            auto it = impl::backendMap.find(backendRes.value());
            if (it == impl::backendMap.end()) {
                jsonLogger()->error("Error while parsing camera_configuration.backend: '{}' is not a valid backend",it->first);
                return WFResult<CameraConfiguration>::failure(JSON_SCHEMA_VIOLATION);
            }
            backend = it->second;
        } else {
            return WFResult<CameraConfiguration>::failure(backendRes.status());
        }

        // Stream format
        StreamFormat format;
        if (auto formatResult = StreamFormat::fromJSON(jobject["format"])) {
            format = std::move(formatResult.value());
        } else {
            jsonLogger()->error("Error while parsing camera_configuration.format");
            return WFResult<CameraConfiguration>::failure(formatResult.status());
        }

        // control aliases
        std::unordered_map<CamControl,std::string> controlAliases;
        if (!jobject["controlAliases"].is_object()) {
            jsonLogger()->error("camera_configuration.controlAliases is not an object");
            return WFResult<CameraConfiguration>::failure(JSON_INVALID_TYPE);
        }
        for (const auto& [control,alias] : jobject["controlAliases"].items()) {
            auto it = impl::camControlMap.find(control);
            if (it == impl::camControlMap.end()) {
                jsonLogger()->error("'{}' is not a recognized camera control",control);
                return WFResult<CameraConfiguration>::failure(JSON_SCHEMA_VIOLATION);
            }
            if (auto astrres = jsonCast<std::string>(alias,std::format("camera_configuration.controlAliases.{}",control))) {
                controlAliases.emplace(it->second,std::move(astrres.value()));
            } else {
                return WFResult<CameraConfiguration>::failure(astrres.status());
            }
        }

        std::vector<CameraIntrinsics> calibvecs;
        // calibrations
        if (jobject.contains("calibrations")) {
            if (!jobject["calibrations"].is_array()) {
                jsonLogger()->error("camera_configuration.calibrations is not an array");
                return WFResult<CameraConfiguration>::failure(JSON_INVALID_TYPE);
            }
            for (const auto& calib_jobject : jobject["calibrations"]) {
                if (auto calibres = CameraIntrinsics::fromJSON(calib_jobject)) {
                    calibvecs.emplace_back(std::move(calibres.value()));
                } else {
                    return WFResult<CameraConfiguration>::failure(calibres.status());
                }
            }
        }

        std::unordered_map<CamControl,int> controls;
        if(jobject.contains("controls")) {
            if (!jobject["controls"].is_object()) {
                jsonLogger()->error("camera_configuration.controls is not an object");
                return WFResult<CameraConfiguration>::failure(JSON_INVALID_TYPE);
            }
            for (const auto& [control,value] : jobject["controls"].items()) {
                auto it = impl::camControlMap.find(control);
                if (it == impl::camControlMap.end()) {
                    jsonLogger()->error("'{}' is not a recognized camera control",control);
                    return WFResult<CameraConfiguration>::failure(JSON_SCHEMA_VIOLATION);
                }
                if (auto valres = jsonCast<int>(value,std::format("camera_configuration.controlAliases.{}",control))) {
                    controls.emplace(it->second,valres.value());
                } else {
                    return WFResult<CameraConfiguration>::failure(valres.status());
                }
            }
        }

        return WFResult<CameraConfiguration>::success(
            std::in_place,
            std::move(devpath),
            backend,
            std::move(format),
            std::move(controlAliases),
            std::move(calibvecs),
            std::move(controls)
        );
    }
}