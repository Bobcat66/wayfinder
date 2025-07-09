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

#include "wfcore/video/FrameConsumer.h"
#include <string>
#include <cscore_oo.h>
#include <cscore_cv.h>

namespace wf {
    class MJPEGVideoServerCS : public FrameConsumer {
    public:
        MJPEGVideoServerCS(StreamFormat format_, std::string name_, int port_);
        void acceptFrame(cv::Mat& data, FrameMetadata meta) noexcept override;
        const std::string& getName() const noexcept override;
        const StreamFormat& getStreamFormat() const noexcept override;
        int setStreamFormat(StreamFormat newformat) override;
    private:
        StreamFormat format;
        cs::CvSource source;
        cs::MjpegServer server;
        int port;
        std::string name;
        cs::VideoMode::PixelFormat cs_pfmt;
    };
}
