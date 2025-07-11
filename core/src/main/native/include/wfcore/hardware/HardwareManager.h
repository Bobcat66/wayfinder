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

#include <unordered_map>
#include <filesystem>
#include <string>
#include "wfcore/video/video_types.h"
#include "wfcore/video/FrameProvider.h"
#include "wfcore/hardware/CameraHandler.h"
#include <memory>
#include <optional>

namespace wf {
    class HardwareManager {
    public:
        // TODO: Refactor exception handling
        int registerCamera(const CameraConfiguration& config);

        bool cameraRegistered(const std::string& devpath) const noexcept;

        CameraBackend getBackend(const std::string& devpath) const;

        FrameProvider& getFrameProvider(const std::string& devpath, const std::string& name);

        void setStreamFormat(const std::string& devpath, const StreamFormat& format);

        StreamFormat getStreamFormat(const std::string& devpath);

        std::optional<CameraIntrinsics> getIntrinsics(const std::string& devpath);

        void setControl(const std::string& devpath, CamControl control, int value);

        int getControl(const std::string& devpath, CamControl control);

        const std::unordered_set<CamControl>& getSupportedControls(const std::string& devpath);

        // TODO: Add method to query supported stream formats

    private:
        const std::unique_ptr<CameraHandler>& getCCamera(const std::string& devpath) const;
        std::unique_ptr<CameraHandler>& getCamera(const std::string& devpath);
        std::unordered_map<std::string,std::unique_ptr<CameraHandler>> cameras;
    };
}