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
#include "wfcore/common/logging.h"
#include <format>
#include <unordered_map>

    // TODO: Make the status logic threadsafe
namespace wf {
    static loggerPtr logger = LoggerManager::getInstance().getLogger("CSCameraSink");

    CSCameraSink::CSCameraSink(std::string name_,std::string devPath_, cs::CvSink appSink_, StreamFormat format_)
    : appSink(appSink_), devPath(devPath_), name(name_)
    , format(format_.fps,{ImageEncoding::BGR24,format_.frameFormat.rows,format_.frameFormat.cols}) {}

    FrameMetadata CSCameraSink::getFrame(cv::Mat& data) noexcept {
        WF_DEBUGLOG(logger,"Acquiring mutex");
        std::lock_guard<std::mutex> lock(camera_guard);
        WF_DEBUGLOG(logger,"Grabbing frame from CSCore");
        uint64_t captimeMicros = appSink.GrabFrame(data);
        WF_DEBUGLOG(logger,"Grabbed frame from CSCore at time {}",captimeMicros);
        return {captimeMicros,format.frameFormat};
    }

    const StreamFormat& CSCameraSink::getStreamFormat() const noexcept {
        std::lock_guard<std::mutex> lock(camera_guard);
        return format;
    }

    // Lowkey the stream format code for CS sinks is really scuffed but whatever.
    // CSCore CVSinks does preprocessing to convert frames to BGR24. Eventually I want to refactor this to use VideoSinks directly, as I have my own preprocessing infrastructure
    // TODO: Test if CvSink converts grayscale images to Y8 and not BGR. Also test depth cameras to see if they output Y16
    void CSCameraSink::setStreamFormat(StreamFormat format) {
        std::lock_guard<std::mutex> lock(camera_guard);
        this->format = StreamFormat(format.fps,{ImageEncoding::BGR24,format.frameFormat.rows,format.frameFormat.cols});
    }

}