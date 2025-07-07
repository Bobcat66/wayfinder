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
#include <memory>
#include <vector>
#include <string>

namespace wf {

    enum class CameraBackend {
        CSCORE,
        REALSENSE, //WIP
        GSTREAMER, //WIP
        LIBCAMERA, //WIP
    };

    enum class CamControl {
        EXPOSURE,
        BRIGHTNESS,
        ISO,
        SHUTTER,
        FOCUS,
        ZOOM,
        WHITE_BALANCE,
        SHARPNESS,
        SATURATION,
        CONTRAST,
        GAMMA,
        HUE
    };

    class CameraHandler {
    public:
        virtual ~CameraHandler() noexcept = default;
        virtual const CameraBackend& getBackend() const noexcept = 0;
        virtual FrameProvider& getFrameProvider() = 0;
        virtual int setFPS(int fps) = 0;
        virtual int setResolution(int width, int height) = 0;
        virtual int setColorSpace(ColorSpace cspace) = 0;
        virtual int setControl(CamControl control, double value) = 0;
        virtual std::vector<CamControl> getControls() = 0;
    };
}