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
#include "wfcore/common/logging.h"

namespace wf {
    using enum WFStatus;

    // This is only for TEMPORARY VIEWS
    const CameraHandler* HardwareManager::getCamera_(const std::string& nickname) const {
        auto it = cameras.find(nickname);
        if (it == cameras.end()) {
            // The camera's presence should be checked externally
            WF_DEBUGLOG(logger(),"Camera {} not found",nickname);
            return nullptr;
        }
        return it->second.get();
    }

    // This is only for TEMPORARY VIEWS
    CameraHandler* HardwareManager::getCamera_(const std::string& nickname) {
        auto it = cameras.find(nickname);
        if (it == cameras.end()) {
            // The camera's presence should be checked externally
            WF_DEBUGLOG(logger(),"Camera {} not found",nickname);
            return nullptr;
        }
        return it->second.get();
    }

    bool HardwareManager::cameraRegistered_impl_(const std::string& nickname) const noexcept {
        auto it = cameras.find(nickname);
        return (it != cameras.end());
    }

    // TODO: Move error handling to exceptions for consistency
    WFStatusResult HardwareManager::registerCamera(const CameraConfiguration& config) {
        std::unique_lock lock(cameras_mtx);
        switch (config.backend) {
            case CameraBackend::CSCORE:
                {   
                    try {
                        auto handlerPtr = CSCameraHandler::create(config);
                        cameras.emplace(config.nickname,std::move(handlerPtr));
                        return WFStatusResult::success();
                    } catch (const wfexception& e) {
                        logger()->error(e.what());
                        return WFStatusResult::failure(e.status());
                    }
                }
            case CameraBackend::REALSENSE: 
                logger()->error("Realsense is not supported yet. Failed to register camera {}.",config.nickname);
                return WFStatusResult::failure(NOT_IMPLEMENTED);
            case CameraBackend::LIBCAMERA:
                logger()->error("Libcamera is not supported yet. Failed to register camera {}.",config.nickname);
                return WFStatusResult::failure(NOT_IMPLEMENTED);
            case CameraBackend::GSTREAMER: 
                logger()->error("GStreamer is not supported yet. Failed to register camera {}.",config.nickname);
                return WFStatusResult::failure(NOT_IMPLEMENTED);
            default: 
                logger()->error("Unrecognized backend specified. Failed to register camera {}",config.nickname);
                return WFStatusResult::failure(HARDWARE_BAD_BACKEND);
        }
    }

    bool HardwareManager::cameraRegistered(const std::string& nickname) const noexcept {
        std::shared_lock lock(cameras_mtx);
        return cameraRegistered_impl_(nickname);
    }

    WFResult<CameraBackend> HardwareManager::getBackend(const std::string& nickname) const {
        std::shared_lock lock(cameras_mtx);
        if (!cameraRegistered_impl_(nickname)) {
            logger()->warn("Camera '{}' is not registered",nickname);
            return WFResult<CameraBackend>::failure(HARDWARE_BAD_CAMERA);
        }
        return getCamera_(nickname)->getBackend();
    }

    WFResult<std::shared_ptr<FrameProvider>> HardwareManager::getFrameProvider(const std::string& nickname, const std::string& name) {
        std::shared_lock lock(cameras_mtx);
        if (!cameraRegistered_impl_(nickname)) {
            logger()->warn("Camera '{}' is not registered",nickname);
            return WFResult<std::shared_ptr<FrameProvider>>::failure(HARDWARE_BAD_CAMERA);
        }
        if (auto res = getCamera_(nickname)->getFrameProvider(name)) {
            return res;
        } else {
            logger()->error(res.what());
            return res;
        }
    }

    WFStatusResult HardwareManager::setStreamFormat(const std::string& nickname, const StreamFormat& format) {
        std::shared_lock lock(cameras_mtx);
        if (!cameraRegistered_impl_(nickname)) {
            logger()->warn("Camera '{}' is not registered",nickname);
            return WFStatusResult::failure(HARDWARE_BAD_CAMERA);
        }
        if (auto res = getCamera_(nickname)->setStreamFormat(format)) {
            return res;
        } else {
            logger()->error(res.what());
            return res;
        }
    }

    WFResult<StreamFormat> HardwareManager::getStreamFormat(const std::string& nickname) {
        std::shared_lock lock(cameras_mtx);
        if (!cameraRegistered_impl_(nickname)) {
            logger()->warn("Camera '{}' is not registered",nickname);
            return WFResult<StreamFormat>::failure(HARDWARE_BAD_CAMERA);
        }
        return getCamera_(nickname)->getStreamFormat();
    }

    WFResult<CameraIntrinsics> HardwareManager::getIntrinsics(const std::string& nickname) {
        std::shared_lock lock(cameras_mtx);
        if (!cameraRegistered_impl_(nickname)) {
            logger()->warn("Camera '{}' is not registered",nickname);
            return WFResult<CameraIntrinsics>::failure(HARDWARE_BAD_CAMERA);
        }
        auto opt = getCamera_(nickname)->getIntrinsics();
        return opt 
            ? WFResult<CameraIntrinsics>::success(std::move(opt.value()))
            : WFResult<CameraIntrinsics>::failure(HARDWARE_NO_CALIB);
    }

    WFStatusResult HardwareManager::setControl(const std::string& nickname, CamControl control, int value) {
        std::shared_lock lock(cameras_mtx);
        if (!cameraRegistered_impl_(nickname)) {
            logger()->warn("Camera '{}' is not registered",nickname);
            return WFStatusResult::failure(HARDWARE_BAD_CAMERA);
        }
        if (auto res = getCamera_(nickname)->setControl(control,value)) {
            return res;
        } else {
            logger()->error(res.what());
            return res;
        }
    }

    WFResult<int> HardwareManager::getControl(const std::string& nickname, CamControl control) {
        std::shared_lock lock(cameras_mtx);
        if (!cameraRegistered_impl_(nickname)) {
            logger()->warn("Camera '{}' is not registered",nickname);
            return WFResult<int>::failure(HARDWARE_BAD_CAMERA);
        }
        if (auto res = getCamera_(nickname)->getControl(control)) {
            return res;
        } else {
            logger()->error(res.what());
            return res;
        }
    }

    WFResult<const std::unordered_set<CamControl>*> HardwareManager::getSupportedControls(const std::string& nickname) {
        std::shared_lock lock(cameras_mtx);
        if (!cameraRegistered_impl_(nickname)) {
            logger()->warn("Camera '{}' is not registered",nickname);
            return WFResult<const std::unordered_set<CamControl>*>::failure(HARDWARE_BAD_CAMERA);
        }
        return getCamera_(nickname)->getSupportedControls();
    }

    WFResult<const std::vector<StreamFormat>*> HardwareManager::getSupportedFormats(const std::string& nickname) {
        std::shared_lock lock(cameras_mtx);
        if (!cameraRegistered_impl_(nickname)) {
            logger()->warn("Camera '{}' is not registered",nickname);
            return WFResult<const std::vector<StreamFormat>*>::failure(HARDWARE_BAD_CAMERA);
        }
        return getCamera_(nickname)->getSupportedFormats();
    }

    WFResult<CameraConfiguration> HardwareManager::getCameraConfiguration(const std::string& nickname) {
        std::shared_lock lock(cameras_mtx);
        if (!cameraRegistered_impl_(nickname)) {
            logger()->warn("Camera '{}' is not registered",nickname);
            return WFResult<CameraConfiguration>::failure(HARDWARE_BAD_CAMERA);
        }
        return getCamera_(nickname)->getConfiguration();
    }

    void HardwareManager::periodic() noexcept {
        std::shared_lock lock(cameras_mtx);
        for(auto it = cameras.begin(); it != cameras.end(); ++it) {
            it->second->periodic();
        }
    }

    bool HardwareManager::cameraHasFaults(const std::string& nickname) const {
        std::shared_lock lock(cameras_mtx);
        if (!cameraRegistered_impl_(nickname)) return true;
        return !(getCamera_(nickname)->ok());
    }
}