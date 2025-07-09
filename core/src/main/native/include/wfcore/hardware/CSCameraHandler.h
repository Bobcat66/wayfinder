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

#include "wfcore/hardware/CameraHandler.h"
#include "wfcore/video/CSCameraSink.h"
#include <unordered_set>
#include <cscore_oo.h>
#include <cscore_cv.h>

namespace wf {
    class CSCameraHandler : public CameraHandler {
    public:
        CSCameraHandler(const CameraConfiguration& config);

        CameraBackend getBackend() const noexcept { return CameraBackend::CSCORE; }

        FrameProvider& getFrameProvider(const std::string& name) override;

        int setStreamFormat(const StreamFormat& format) override;

        const std::unordered_set<CamControl>& getSupportedControls() override { return supportedControls; }

        const std::vector<StreamFormat>& getSupportedFormats() override { return supportedFormats; }

        const StreamFormat& getStreamFormat() override;

        std::optional<CameraIntrinsics> getIntrinsics() override;

        void setControl(CamControl control, int value) override;

        int getControl(CamControl control) override;

        int getError() override { return error; }
    private:
        std::unordered_map<std::string,CSCameraSink> sinks;
        cs::UsbCamera camera;
        std::string devpath;
        StreamFormat format;
        std::vector<StreamFormat> supportedFormats;
        std::vector<cv::Size> calibratedResolutions;
        std::vector<CameraIntrinsics> calibrations;
        std::unordered_set<CamControl> supportedControls;
        std::unordered_map<CamControl,std::string> controlAliases;
        int error = 0;
    };
}