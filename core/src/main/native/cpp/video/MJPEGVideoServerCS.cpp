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

#include "wfcore/video/MJPEGVideoServerCS.h"
#include "wfcore/video/video_utils.h"
#include <format>

namespace wf {
    MJPEGVideoServerCS::MJPEGVideoServerCS(StreamFormat format_, std::string name_, int port_)
    : format(format_), name(std::move(name_)), port(port_), cs_pfmt(getPixelFormatFromEncoding(format.frameFormat.encoding))
    , source(
        std::format("{}_source",name),
        cs_pfmt,
        format.frameFormat.width,
        format.frameFormat.height,
        format.fps
    )
    , server(
        std::format("{}_server",name),
        port
    ) {
        server.SetSource(source);
    }
    void MJPEGVideoServerCS::acceptFrame(cv::Mat& data, FrameMetadata meta) noexcept {
        source.PutFrame(data,cs_pfmt,false);
    }
    const std::string& MJPEGVideoServerCS::getName() const noexcept {
        return name;
    }
    const StreamFormat& MJPEGVideoServerCS::getStreamFormat() const noexcept {
        return format;
    }
    int MJPEGVideoServerCS::setStreamFormat(StreamFormat newformat) {
        format = newformat;
        cs_pfmt = getPixelFormatFromEncoding(format.frameFormat.encoding);
        if (!source.SetPixelFormat(cs_pfmt)) return 1;
        if (!source.SetResolution(format.frameFormat.width,format.frameFormat.height)) return 2;
        if (!source.SetFPS(format.fps)) return 3;
        return 0;
    }
}