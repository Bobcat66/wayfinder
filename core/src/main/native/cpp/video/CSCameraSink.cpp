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


#include "wfcore/video/CSCameraSink.h"
#include <format>
#include <unordered_map>

namespace wf {

    CSCameraSink::CSCameraSink(std::string name_,std::string devPath_, cs::CvSink appSink_, StreamFormat format_)
    : appSink(appSink_), devPath(devPath_), name(name_), format(format_) {}

    FrameMetadata CSCameraSink::getFrame(cv::Mat& data) noexcept {
        std::lock_guard<std::mutex> lock(camera_guard);
        uint64_t captimeMicros = appSink.GrabFrame(data);
        return {captimeMicros,getStreamFormat().frameFormat};
    }

    const StreamFormat& CSCameraSink::getStreamFormat() const noexcept {
        std::lock_guard<std::mutex> lock(camera_guard);
        return format;
    }

    void CSCameraSink::setStreamFormat(StreamFormat format) {
        std::lock_guard<std::mutex> lock(camera_guard);
        this->format = format;
    }

    

}