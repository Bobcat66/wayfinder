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
#include <cscore_raw.h>
#include <cscore_cpp.h>
#include <wpi/RawFrame.h>
#include <memory>
#include <mutex>
#include "wfcore/common/wfdef.h"
// A CSCore-based implementation of FrameProvider, which pulls frames from a USB camera 
namespace wf {
    class CSCameraHandler;

    class CSCameraSink : public FrameProvider, private cs::RawSink {
        friend class CSCameraHandler;
    public:
        CSCameraSink(std::shared_ptr<CSCameraHandler> handler, std::string name);
        FrameMetadata getFrame(cv::Mat& data) noexcept override;
        std::string getName() const override;
        WFResult<std::string> getDevPath() const;
        WFResult<StreamFormat> getStreamFormat() const noexcept override;
    private:
        void acquireSource(std::shared_ptr<CSCameraHandler>& handler);
        //mutable std::mutex camera_guard_;
        std::weak_ptr<CSCameraHandler> handler_;
        wpi::RawFrame raw_buffer_;
    };
}