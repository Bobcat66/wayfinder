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
#include <stdexcept>
#include <format>

namespace wf {
    // TODO: Add verification
    CSCameraHandler::CSCameraHandler(const CameraConfiguration& config) 
    : devpath(config.devpath)
    , format(config.format)
    , calibratedResolutions(config.calibratedResolutions)
    , calibrations(config.calibrations)
    , controlAliases(config.controlAliases)
    , camera(std::format("{}_source",devpath),devpath) {
        auto videomodes = camera.EnumerateVideoModes();
        for (const auto& videomode : videomodes) {
            supportedFormats.push_back(getStreamFormatFromVideoMode(videomode));
        }
        for (const auto& entry : config.controlAliases) {
            supportedControls.insert(entry.first);
        }
        if (!camera.SetVideoMode(getVideoModeFromStreamFormat(format))) {
            this->error = 3; // 3 denotes invalid format
        }
    }

    FrameProvider& CSCameraHandler::getFrameProvider(const std::string& name){
        auto it = sinks.find(name);
        if (it != sinks.end()) {
            return it->second;
        }
        cs::CvSink appsink;
        appsink.SetSource(camera);
        auto emplaced = sinks.emplace(name,CSCameraSink{name,devpath,std::move(appsink),format});
        // TODO: Check if operation was successful
        return emplaced.first->second;
    }

    int CSCameraHandler::setStreamFormat(const StreamFormat& format) {
        this->format = format;
        for (auto& pair : this->sinks) {
            pair.second.setStreamFormat(this->format);
        }
        return 0;
    }

    const StreamFormat& CSCameraHandler::getStreamFormat() {
        return this->format;
    }

    std::optional<CameraIntrinsics> CSCameraHandler::getIntrinsics() {
        cv::Size res(
            this->format.frameFormat.cols,
            this->format.frameFormat.rows
        );
        for (int i = 0 ; i < calibratedResolutions.size() ; ++i) {
            if (calibratedResolutions[i] == res) {
                return std::make_optional(calibrations[i]);
            }
        }
        return std::nullopt;
    }
    
    void CSCameraHandler::setControl(CamControl control, int value) {
        auto it = controlAliases.find(control);
        if (it == controlAliases.end()) {
            // Control has no alias, give up
            this->error = 1;
            return; // Invalid control
        }
        auto property = camera.GetProperty(it->second);
        if (property.GetKind() == cs::VideoProperty::Kind::kNone) {
            // No property with the aliased name exists, give up
            this->error = 1;
            return;
        }
        property.Set(value);
        // Check that the operation was successful
        if (property.Get() != value) this->error = 2; // 2 denotes unknown error
        return;
    }

    int CSCameraHandler::getControl(CamControl control) {
        auto it = controlAliases.find(control);
        if (it == controlAliases.end()) {
            // Control has no alias, give up
            this->error = 1;
            return 0; // Invalid control
        }
        auto property = camera.GetProperty(it->second);
        if (property.GetKind() == cs::VideoProperty::Kind::kNone) {
            // No property with the aliased name exists, give up
            this->error = 1;
            return 0;
        }
        return property.Get();
    }
}