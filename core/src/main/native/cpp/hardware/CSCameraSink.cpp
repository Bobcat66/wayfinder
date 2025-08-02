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


#include "wfcore/hardware/CSCameraSink.h"
#include "wfcore/hardware/CSCameraHandler.h"
#include "wfcore/common/logging.h"
#include <format>
#include <unordered_map>
#include "wfcore/video/video_utils.h"
#include "wfcore/common/wfassert.h"


namespace wf {
    using enum WFStatus;
    static loggerPtr logger = LoggerManager::getInstance().getLogger("CSCameraSink");

    CSCameraSink::CSCameraSink(std::shared_ptr<CSCameraHandler> handler, std::string name)
    : cs::RawSink(name), handler_(handler) {
        acquireSource(handler);
    }

    FrameMetadata CSCameraSink::getFrame(cv::Mat& data) noexcept {
        auto locked = handler_.lock();
        if (!locked) return FrameMetadata::badFrame(BAD_ACQUIRE);
        if (!locked->ok()) return FrameMetadata::badFrame(locked->getStatus());

        auto micros = GrabFrame(raw_buffer_);
        if (!micros) {
            // A camera fault was not detected by the Hardware Manager, report it here
            logger->error("CSCore error from sink {}: {}",GetName(),GetError());
            return FrameMetadata::badFrame(HARDWARE_CSCORE);
        }

        auto format = locked->getStreamFormat().frameFormat;
        int cvType = getCVTypeFromEncoding(format.encoding);
        WF_Assert(format.height == raw_buffer_.height && format.width == raw_buffer_.width);

        data.create(format.height, format.width, cvType);

        WF_Assert(data.step == raw_buffer_.stride);
        size_t totalBytes = format.height * raw_buffer_.stride;
        WF_Assert(data.isContinuous());
        std::memcpy(data.data, raw_buffer_.data, totalBytes);

        return FrameMetadata(micros, format);
    }

    WFResult<StreamFormat> CSCameraSink::getStreamFormat() const noexcept {
        auto locked = handler_.lock();
        return locked 
            ? WFResult<StreamFormat>::success(locked->getStreamFormat())
            : WFResult<StreamFormat>::failure(BAD_ACQUIRE);
    }

    WFResult<std::string> CSCameraSink::getDevPath() const {
        auto locked = handler_.lock();
        return locked 
            ? WFResult<std::string>::success(locked->getDevPath())
            : WFResult<std::string>::failure(BAD_ACQUIRE);
    }

    std::string CSCameraSink::getName() const {
        return GetName();
    }

    void CSCameraSink::acquireSource(std::shared_ptr<CSCameraHandler>& handler) {
        this->SetSource(handler->camera_);
    }

}