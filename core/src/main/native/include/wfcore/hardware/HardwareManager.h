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

namespace wf {
    class HardwareManager {
    public:
        // TODO: Refactor instantiation exception handling
        int registerCamera(const CameraConfiguration& config);

        CameraBackend getBackend(const std::string& devpath) const noexcept;

        FrameProvider& getFrameProvider(const std::string& devpath, const std::string& name);

        int setStreamFormat(const std::string& devpath, const StreamFormat& format);

        StreamFormat getStreamFormat(const std::string& devpath);

        std::optional<CameraIntrinsics> getIntrinsics(const std::string& devpath);

        int setControl(const std::string& devpath, CamControl control, int value, bool persist);

        double getControl(const std::string& devpath, CamControl control);

        const std::unordered_set<CamControl>& getControls(const std::string& devpath);
    private:
        std::unordered_map<std::string,std::unique_ptr<CameraHandler>> cameras;
        std::unordered_map<std::string,std::filesystem::path> cameraConfigs; // Path to camera configurations. These can be written
    };
}