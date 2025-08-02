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
#include "wfcore/hardware/CSCameraSink.h"
#include <unordered_set>
#include <cscore_oo.h>
#include <cscore_cv.h>

namespace wf {

    class CSCameraHandler : public CameraHandler, private std::enable_shared_from_this<CSCameraHandler> {
        friend void CSCameraSink::acquireSource(std::shared_ptr<CSCameraHandler>& handler);
    public:

        CameraBackend getBackend() const noexcept { return CameraBackend::CSCORE; }

        WFResult<std::shared_ptr<FrameProvider>> getFrameProvider(const std::string& name) override;

        std::string getDevPath() const { return devpath_; }

        std::string getNickname() const override { return name_; }

        WFStatusResult setStreamFormat(const StreamFormat& format) override;

        const std::unordered_set<CamControl>* getSupportedControls() override { return &supportedControls_; }

        const std::vector<StreamFormat>* getSupportedFormats() override { return &supportedFormats_; }

        StreamFormat getStreamFormat() override;

        // Performs periodic system health checks and monitoring
        void periodic() override;

        std::optional<CameraIntrinsics> getIntrinsics() override;

        WFStatusResult setControl(CamControl control, int value) override;

        WFResult<int> getControl(CamControl control) override;

        CameraConfiguration getConfiguration() override;

        static std::shared_ptr<CSCameraHandler> create(const CameraConfiguration& config) {
            return std::shared_ptr<CSCameraHandler>(new CSCameraHandler(config));
        }

    private:
        CSCameraHandler(const CameraConfiguration& config);
        std::unordered_map<std::string,std::weak_ptr<CSCameraSink>> sinks_;
        cs::UsbCamera camera_;
        std::string name_;
        std::string devpath_;
        StreamFormat format_;
        std::vector<StreamFormat> supportedFormats_;
        std::vector<CameraIntrinsics> calibrations_;
        std::unordered_set<CamControl> supportedControls_;
        std::unordered_map<CamControl,std::string> controlAliases_;
        std::unordered_map<CamControl,int> controls_;
    };
}