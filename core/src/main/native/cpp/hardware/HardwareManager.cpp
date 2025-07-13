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

#include "wfcore/hardware/HardwareManager.h"
#include "wfcore/hardware/CSCameraHandler.h"
#include "wfcore/common/logging/LoggerManager.h"

namespace wf {

    static loggerPtr logger = LoggerManager::getInstance().getLogger("HardwareManager",LogGroup::Config);

    // Tries to get a camera, throws exception if it doesn't exist
    const std::unique_ptr<CameraHandler>& HardwareManager::getCCamera(const std::string& devpath) const {
        auto it = cameras.find(devpath);
        if (it == cameras.end()) {
            // TODO: refactor this
            throw std::runtime_error(std::format("Camera {} does not exist!",devpath));
        }
        return it->second;
    }

    std::unique_ptr<CameraHandler>& HardwareManager::getCamera(const std::string& devpath) {
        auto it = cameras.find(devpath);
        if (it == cameras.end()) {
            // TODO: refactor this
            throw std::runtime_error(std::format("Camera {} does not exist!",devpath));
        }
        return it->second;
    }

    // TODO: Move error handling to exceptions for consistency
    int HardwareManager::registerCamera(const CameraConfiguration& config) {
        switch (config.backend) {
            case CameraBackend::CSCORE:
                {
                    auto handlerPtr = std::make_unique<CSCameraHandler>(config);
                    if (handlerPtr->getError()) {
                        throw std::runtime_error(std::format("Error: Invalid cscore configuration for {}, Error code {}",config.devpath,handlerPtr->getError())); // TODO, make this better
                    }
                    cameras.emplace(config.devpath,std::move(handlerPtr));
                    return 0;
                }
            case CameraBackend::REALSENSE: 
                logger->warn("Realsense is not supported yet. Camera {} not configured",config.devpath);
                return 2;
            case CameraBackend::LIBCAMERA:
                logger->warn("Realsense is not supported yet. Camera {} not configured",config.devpath);
                return 2;
            case CameraBackend::GSTREAMER: 
                logger->warn("Realsense is not supported yet. Camera {} not configured",config.devpath);
                return 2;
            default: 
                logger->error("Unrecognized backend specified for {}",config.devpath);
                return 3;
        }
    }

    bool HardwareManager::cameraRegistered(const std::string& devpath) const noexcept {
        auto it = cameras.find(devpath);
        return (it != cameras.end());
    }

    CameraBackend HardwareManager::getBackend(const std::string& devpath) const {
        return getCCamera(devpath)->getBackend();
    }

    FrameProvider& HardwareManager::getFrameProvider(const std::string& devpath, const std::string& name) {
        return getCamera(devpath)->getFrameProvider(name);
    }

    void HardwareManager::setStreamFormat(const std::string& devpath, const StreamFormat& format) {
        auto err = getCamera(devpath)->setStreamFormat(format);
        if (err) {
            throw std::runtime_error(std::format("Attempted to configure {} with invalid stream format!",devpath));
        }
    }

    StreamFormat HardwareManager::getStreamFormat(const std::string& devpath) {
        return getCamera(devpath)->getStreamFormat();
    }

    std::optional<CameraIntrinsics> HardwareManager::getIntrinsics(const std::string& devpath) {
        return getCamera(devpath)->getIntrinsics();
    }

    void HardwareManager::setControl(const std::string& devpath, CamControl control, int value) {
        getCamera(devpath)->setControl(control,value);
    }

    int HardwareManager::getControl(const std::string& devpath, CamControl control) {
        return getCamera(devpath)->getControl(control);
    }

    const std::unordered_set<CamControl>& HardwareManager::getSupportedControls(const std::string& devpath) {
        return getCamera(devpath)->getSupportedControls();
    }
}