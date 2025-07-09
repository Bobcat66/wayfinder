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
#include <cscore_oo.h>
#include <cscore_cv.h>
#include <memory>
#include <mutex>

// A CSCore-based implementation of FrameProvider, which pulls frames from a USB camera 
namespace wf {
    class CSCameraHandler;

    class CSCameraSink : public FrameProvider {
        friend class CSCameraHandler;
    public:
        CSCameraSink(std::string name_,std::string devPath_, cs::CvSink appSink_, StreamFormat format_);
        FrameMetadata getFrame(cv::Mat& data) noexcept override;
        const std::string& getName() const noexcept override { return name; }
        const std::string& getDevPath() const noexcept { return devPath; }
        const StreamFormat& getStreamFormat() const noexcept override;
    private:
        void setStreamFormat(StreamFormat format);
        mutable std::mutex camera_guard;
        cs::CvSink appSink;
        std::string devPath;
        std::string name;
        StreamFormat format;
    };
}