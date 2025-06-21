/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of Wayfinder.
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
#include <cscore_oo.h>
#include <cscore_cv.h>
#include <memory>

// A CSCore-based implementation of FrameProvider, which pulls frames from a USB camera 
namespace wf {
    class UsbCameraSinkCS : public FrameProvider {
    public:
        UsbCameraSinkCS(const std::string& name, const std::string& devPath);
        ~UsbCameraSinkCS() override = default;
        Frame getFrame() noexcept override;
        const std::string& getName() const noexcept override;
        const std::string& getDevPath() const noexcept;
        StreamFormat getStreamFormat() const noexcept;
    private:
        cs::CvSink appSink;
        std::string devPath;
        std::string name;
        std::shared_ptr<cs::UsbCamera> camera;
    };

    bool setUsbCameraFormat(const std::string& devPath, StreamFormat format);
}