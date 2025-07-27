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

#define CAMCONTROL_PATTERN R"(^(EXPOSURE|AUTO_EXPOSURE|BRIGHTNESS|ISO|SHUTTER|FOCUS|ZOOM|WHITE_BALANCE|AUTO_WHITE_BALANCE|SHARPNESS|SATURATION|CONTRAST|GAMMA|HUE)$)"

// TODO: refactor to use getProperty();
namespace impl {

    using namespace wf;

    cv::Mat createIntrinsicsMatrix(double fx, double fy, double cx, double cy) {
        return (cv::Mat_<double>(3, 3) <<
            fx,  0, cx,
             0, fy, cy,
             0,  0,  1);
    }

    static const JSONValidationFunctor* getBackendValidator() {
        static JSONEnumValidator validator({
            "CSCORE",
            "REALSENSE",
            "GSTREAMER",
            "LIBCAMERA"
        });
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    static const JSONValidationFunctor* getResolutionValidator() {
        static JSONStructValidator validator(
            // Properties
            {
                {"width", getPrimitiveValidator<int>()},
                {"height", getPrimitiveValidator<int>()}
            },
            // Required properties
            {"width","height"}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    static const JSONValidationFunctor* getMatrixValidator() {
        static JSONStructValidator validator(
            {
                {"fx", getPrimitiveValidator<double>()},
                {"fy", getPrimitiveValidator<double>()},
                {"cx", getPrimitiveValidator<double>()},
                {"cy", getPrimitiveValidator<double>()}
            },
            {"fx","fy","cx","cy"}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    static const JSONValidationFunctor* getDistortionValidator() {
        static JSONArrayValidator validator(getPrimitiveValidator<double>(),5,8);
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    static const JSONValidationFunctor* getControlAliasValidator() {
        static JSONMapValidator validator(getPrimitiveValidator<std::string>(),CAMCONTROL_PATTERN);
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    static const JSONValidationFunctor* getCalibrationsValidator() {
        static JSONArrayValidator validator(CameraIntrinsics::getValidator());
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    static const JSONValidationFunctor* getControlsValidator() {
        static JSONMapValidator validator(getPrimitiveValidator<int>(),CAMCONTROL_PATTERN);
        return static_cast<JSONValidationFunctor*>(&validator);
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

    const JSONValidationFunctor* CameraIntrinsics::getValidator_impl() {
        static JSONStructValidator validator(
            {
                {"resolution",impl::getResolutionValidator()},
                {"matrix",impl::getMatrixValidator()},
                {"distortion",impl::getDistortionValidator()}
            },
            {"resolution","matrix","distortion"}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }

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

        auto valid = (*getValidator())(jobject);
        if (!valid) return WFResult<CameraIntrinsics>::propagateFail(valid);
        
        return WFResult<CameraIntrinsics>::success(
            std::in_place,
            cv::Size{
                jobject["resolution"]["width"].get<int>(),
                jobject["resolution"]["height"].get<int>()
            },
            impl::createIntrinsicsMatrix(
                jobject["matrix"]["fx"],
                jobject["matrix"]["fy"],
                jobject["matrix"]["cx"],
                jobject["matrix"]["cy"]
            ),
            cv::Mat(jobject["distortion"].get<std::vector<double>>()).reshape(1,1).clone()
        );
    }

    const JSONValidationFunctor* CameraConfiguration::getValidator_impl() {
        static JSONStructValidator validator(
            {
                {"devpath", getPrimitiveValidator<std::string>()},
                {"backend", impl::getBackendValidator()},
                {"format", StreamFormat::getValidator()},
                {"controlAliases",impl::getControlAliasValidator()},
                {"calibrations",impl::getCalibrationsValidator()},
                {"controls",impl::getControlsValidator()}
            },
            {"devpath","backend","format","controlAliases"}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
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
        } catch (const JSON::exception& e) {
            jsonLogger()->error("JSON error while serializing CameraConfiguration: {}",e.what());
            return WFResult<JSON>::failure(JSON_UNKNOWN);
        }
    }

    WFResult<CameraConfiguration> CameraConfiguration::fromJSON_impl(const JSON& jobject) {

        auto valid = (*getValidator())(jobject);
        if (!valid) return WFResult<CameraConfiguration>::propagateFail(valid);

        // control aliases
        std::unordered_map<CamControl,std::string> controlAliases;
        for (const auto& [control,alias] : jobject["controlAliases"].items()) {
            controlAliases[impl::camControlMap.at(control)] = alias.get<std::string>();
        }

        std::vector<CameraIntrinsics> calibvec;
        // calibrations
        if (jobject.contains("calibrations")) {
            for (const auto& calib_jobject : jobject["calibrations"]) {
                if (auto calibres = CameraIntrinsics::fromJSON(calib_jobject)) {
                    calibvec.emplace_back(std::move(calibres.value()));
                } else {
                    return WFResult<CameraConfiguration>::propagateFail(calibres);
                }
            }
        }

        std::unordered_map<CamControl,int> controls;
        if(jobject.contains("controls")) {
            for (const auto& [control,value] : jobject["controls"].items()) {
                controls[impl::camControlMap.at(control)] = value.get<int>();
            }
        }

        StreamFormat format;
        auto fres = StreamFormat::fromJSON(jobject["format"]);
        if (!fres) return WFResult<CameraConfiguration>::propagateFail(fres);
        format = std::move(fres.value());

        return WFResult<CameraConfiguration>::success(
            std::in_place,
            jobject["devpath"].get<std::string>(),
            impl::backendMap.at(jobject["backend"].get<std::string>()),
            std::move(format),
            std::move(controlAliases),
            std::move(calibvec),
            std::move(controls)
        );
    }
}