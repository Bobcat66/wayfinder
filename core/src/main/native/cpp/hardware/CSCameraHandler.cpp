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
    , intrinsics(config.intrinsics) 
    , supportedControls(config.supportedControls) {
        camera(std::format("{}_source",devpath),devpath);
        auto videomodes = camera.EnumerateVideoModes();
        for (const auto& videomode : videomodes) {
            supportedFormats.push_back(
                getStreamFormatFromVideoMode()
            )
        }
        if (!camera.SetVideoMode(getVideoModeFromSformat(format))) {
            throw std::runtime_error("Camera configuration specifies an invalid stream format!");
        }
    }
    FrameProvider& getFrameProvider(const std::string& name){

    }
    int setStreamFormat(const StreamFormat& format) {

    }
    StreamFormat getStreamFormat() {

    }
    std::optional<CameraIntrinsics> getIntrinsics() {

    }
    int setControl(CamControl control, double value) {

    }
    double getControl(CamControl control, double value) {

    }
}