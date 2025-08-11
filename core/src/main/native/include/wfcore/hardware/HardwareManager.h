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
#include "wfcore/common/status.h"
#include <memory>
#include <optional>
#include <shared_mutex>

// Camera Handlers WILL NOT be destroyed once created. They effectively have unlimited lifetime
// This rule applies even if the camera is disconnected or has some other persistent fault.
// They can be deactivated, but never deallocated
// This allows us to access them by reference without worrying about lifetime management
namespace wf {
    class HardwareManager : public WFConcurrentLoggedStatusfulObject {
    public:
        HardwareManager() : WFConcurrentLoggedStatusfulObject("HardwareManager",LogGroup::General) {}
        // TODO: Refactor exception handling
        WFStatusResult registerCamera(const CameraConfiguration& config);

        bool cameraRegistered(const std::string& nickname) const noexcept;

        WFResult<CameraBackend> getBackend(const std::string& nickname) const;

        WFResult<std::shared_ptr<FrameProvider>> getFrameProvider(const std::string& nickname, const std::string& provider_name);

        WFStatusResult setStreamFormat(const std::string& nickname, const StreamFormat& format);

        WFResult<StreamFormat> getStreamFormat(const std::string& nickname);

        WFResult<CameraIntrinsics> getIntrinsics(const std::string& nickname);

        WFStatusResult setControl(const std::string& nickname, CamControl control, int value);

        WFResult<int> getControl(const std::string& nickname, CamControl control);

        WFResult<const std::unordered_set<CamControl>*> getSupportedControls(const std::string& nickname);

        WFResult<const std::vector<StreamFormat>*> getSupportedFormats(const std::string& nickname);

        WFResult<CameraConfiguration> getCameraConfiguration(const std::string& nickname);

        void periodic() noexcept;

        // Returns whether or not the camera with the given name has faults
        bool cameraHasFaults(const std::string& nickname) const;
    private:
        bool cameraRegistered_impl_(const std::string& nickname) const noexcept;
        const CameraHandler* getCamera_(const std::string& nickname) const;
        CameraHandler* getCamera_(const std::string& nickname);
        std::unordered_map<std::string,std::shared_ptr<CameraHandler>> cameras;
        std::shared_mutex cameras_mtx;
    };
}