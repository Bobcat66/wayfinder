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

#include "wfcore/video/FrameProvider.h"
#include "wfcore/video/video_types.h"
#include "wfcore/hardware/CameraConfiguration.h"
#include "wfcore/common/status.h"
#include <memory>
#include <vector>
#include <unordered_set>
#include <string>
#include <optional>

namespace wf {

    class CameraHandler : public WFConcurrentStatusfulObject {
    public:
        virtual ~CameraHandler() noexcept = default;

        virtual CameraBackend getBackend() const noexcept = 0;

        virtual WFResult<std::shared_ptr<FrameProvider>> getFrameProvider(const std::string& name) = 0;

        virtual WFStatusResult setStreamFormat(const StreamFormat& format) = 0;

        virtual StreamFormat getStreamFormat() = 0;

        // Performs periodic system health checks and monitoring
        virtual void periodic() = 0;

        virtual std::optional<CameraIntrinsics> getIntrinsics() = 0;

        virtual WFStatusResult setControl(CamControl control, int value) = 0;

        virtual WFResult<int> getControl(CamControl control) = 0;

        virtual const std::unordered_set<CamControl>* getSupportedControls() = 0;

        virtual const std::vector<StreamFormat>* getSupportedFormats() = 0;

        virtual std::string getNickname() const = 0;

        virtual CameraConfiguration getConfiguration() = 0;
    };
}