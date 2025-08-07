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

#include "wfcore/hardware/CSCameraHandler.h"
#include "wfcore/video/video_utils.h"
#include "wfcore/common/wfexcept.h"
#include <format>

namespace wf {
    using enum WFStatus;
    // TODO: Add verification, Refactor error handling
    CSCameraHandler::CSCameraHandler(const CameraConfiguration& config) 
    : name_(config.nickname)
    , devpath_(config.devpath)
    , format_(config.format)
    , calibrations_(config.calibrations)
    , controlAliases_(config.controlAliases)
    , camera_(std::format("{}_source",devpath_),devpath_) {
        camera_.SetConnectionStrategy(cs::VideoSource::kConnectionKeepOpen);
        auto videomodes = camera_.EnumerateVideoModes();
        for (const auto& videomode : videomodes) {
            supportedFormats_.push_back(getStreamFormatFromVideoMode(videomode));
        }
        for (const auto& entry : config.controlAliases) {
            supportedControls_.insert(entry.first);
        }
        for (const auto& [control,value] : config.controls) {
            auto cres = setControl(control,value);
            if (!cres) throw invalid_camera_control(cres.what());
        }
        if (!camera_.SetVideoMode(getVideoModeFromStreamFormat(format_))) {
            throw invalid_stream_format("Camera {} configured with invalid stream format",config.devpath);
        }
    }

    WFResult<std::shared_ptr<FrameProvider>> CSCameraHandler::getFrameProvider(const std::string& name){
        if (!ok()) return WFResult<std::shared_ptr<FrameProvider>>::failure(getStatus(),getError());
        auto it = sinks_.find(name);
        if (it != sinks_.end()) {
            // If a frame provider with that name already exists in the sink registry and the pointer is valid, return it
            if (auto locked = it->second.lock()) 
                return WFResult<std::shared_ptr<FrameProvider>>::success(std::move(locked));
            // If the pointer is not valid, remove the entry from the sink registry
            sinks_.erase(it);
        }
        cs::CvSink appsink;
        auto provider = std::make_shared<CSCameraSink>(shared_from_this(),name);
        std::weak_ptr<CSCameraSink> provider_registry_ref(provider);
        sinks_.insert({name,provider_registry_ref});
        // TODO: Check if operation was successful
        return WFResult<std::shared_ptr<FrameProvider>>::success(std::move(provider));
    }

    WFStatusResult CSCameraHandler::setStreamFormat(const StreamFormat& format) {
        if (!ok()) return WFStatusResult::failure(getStatus(),getError());
        if (camera_.SetVideoMode(getVideoModeFromStreamFormat(format))) {
            this->format_ = format;
            return WFStatusResult::success();
        } else {
            return WFStatusResult::failure(HARDWARE_BAD_FORMAT);
        }
    }

    StreamFormat CSCameraHandler::getStreamFormat() {
        return this->format_;
    }

    std::optional<CameraIntrinsics> CSCameraHandler::getIntrinsics() {
        cv::Size res(
            this->format_.frameFormat.width,
            this->format_.frameFormat.height
        );
        for (auto calibration : calibrations_) {
            if (calibration.resolution == res) {
                return std::make_optional(calibration);
            }
        }
        return std::nullopt;
    }
    
    WFStatusResult CSCameraHandler::setControl(CamControl control, int value) {
        if (!ok()) return WFStatusResult::failure(getStatus(),getError());
        auto it = controlAliases_.find(control);
        if (it == controlAliases_.end()) {
            // Control has no alias, give up
            return WFStatusResult::failure(
                HARDWARE_BAD_CONTROL,
                "Attempted to change non-aliased camera control for camera {}",devpath_
            );
        }
        auto property = camera_.GetProperty(it->second);
        if (property.GetKind() == cs::VideoProperty::Kind::kNone) {
            // No property with the aliased name exists, give up
            return WFStatusResult::failure(
                HARDWARE_BAD_CONTROL,
                "Attempted to change unsupported camera control for camera {}",devpath_
            );
        }
        property.Set(value);
        // Check that the operation was successful
        if (property.Get() != value){
            return WFStatusResult::failure(HARDWARE_UNKNOWN);
        } else {
            // Cache updated control
            this->controls_[control] = value;
            return WFStatusResult::success();
        }
    }

    WFResult<int> CSCameraHandler::getControl(CamControl control) {
        auto it = controlAliases_.find(control);
        if (it == controlAliases_.end()) {
            // Control has no alias, give up
            return WFResult<int>::failure(
                HARDWARE_BAD_CONTROL,
                "Attempted to query non-aliased camera control for camera {}", devpath_
            );
        }
        auto property = camera_.GetProperty(it->second);
        if (property.GetKind() == cs::VideoProperty::Kind::kNone) {
            // No property with the aliased name exists, give up
            return WFResult<int>::failure(
                HARDWARE_BAD_CONTROL,
                "Attempted to query unsupported camera control for camera {}",devpath_
            );
        }
        return WFResult<int>::success(property.Get());
    }

    void CSCameraHandler::periodic() {
        if (!ok()) return;
        if (!camera_.IsConnected()){
            this->reportError(
                HARDWARE_DISCONNECT,
                "Camera {} ({}) disconnected", name_, devpath_
            );
            return;
        }
    }

    CameraConfiguration CSCameraHandler::getConfiguration() {
        return CameraConfiguration(
            this->name_,
            this->devpath_,
            CameraBackend::CSCORE,
            this->format_,
            this->controlAliases_,
            this->calibrations_,
            this->controls_
        );
    }

    void CSCameraHandler::disable() {
        this->reportError(WFStatus::HARDWARE_DISABLED);
        camera_.SetConnectionStrategy(cs::VideoSource::kConnectionForceClose);
    }
}