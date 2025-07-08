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

#include "wfcore/video/video_types.h"
#include <cscore_oo.h>
#include <opencv2/core.hpp>
#include <stdexcept>

namespace wf {
    [[deprecated]]
    cv::Mat generateEmptyFrameBuf(FrameFormat format);

    int getCVTypeFromEncoding(ImageEncoding encoding) {
        switch (encoding) {
            case ImageEncoding::BGR24: return CV_8UC3;
            case ImageEncoding::RGB24: return CV_8UC3;
            case ImageEncoding::RGB565: return CV_8UC2;
            case ImageEncoding::BGRA: return CV_8UC4;
            case ImageEncoding::RGBA: return CV_8UC4;
            case ImageEncoding::YUYV: return CV_8UC2;
            case ImageEncoding::UYVY: return CV_8UC2;
            case ImageEncoding::Y8: return CV_8UC1;
            case ImageEncoding::Y16: return CV_16UC1;
            default: return 0; // Denotes unknown value
        }
    }

    cs::VideoMode::PixelFormat getPixelFormatFromEncoding(ImageEncoding encoding) {
        switch (encoding) {
            case ImageEncoding::BGR24: return cs::VideoMode::PixelFormat::kBGR;
            case ImageEncoding::BGRA: return cs::VideoMode::PixelFormat::kBGRA;
            case ImageEncoding::YUYV: return cs::VideoMode::PixelFormat::kYUYV;
            case ImageEncoding::UYVY: return cs::VideoMode::PixelFormat::kUYVY;
            case ImageEncoding::Y8: return cs::VideoMode::PixelFormat::kGray;
            case ImageEncoding::Y16: return cs::VideoMode::PixelFormat::kY16;
            case ImageEncoding::MJPEG: return cs::VideoMode::PixelFormat::kMJPEG;
            default: return cs::VideoMode::PixelFormat::kUnknown;
        };
    }

    ImageEncoding getEncodingFromPixelFormat(cs::VideoMode::PixelFormat pixelFormat) {
        switch (pixelFormat) {
            case cs::VideoMode::PixelFormat::kBGR: return ImageEncoding::BGR24;
            case cs::VideoMode::PixelFormat::kBGRA: return ImageEncoding::BGRA;
            case cs::VideoMode::PixelFormat::kYUYV: return ImageEncoding::YUYV;
            case cs::VideoMode::PixelFormat::kUYVY: return ImageEncoding::UYVY;
            case cs::VideoMode::PixelFormat::kGray: return ImageEncoding::Y8;
            case cs::VideoMode::PixelFormat::kY16: return ImageEncoding::Y16;
            case cs::VideoMode::PixelFormat::kMJPEG: return ImageEncoding::MJPEG;
            default: return ImageEncoding::UNKNOWN;
        };
    }

    template <CVImage T>
    inline T generateEmptyCVImg(FrameFormat format) {
        return T{format.rows,format.cols,getCVTypeFromEncoding(format.encoding)};
    }

    cs::VideoMode getVideoModeFromStreamFormat(const StreamFormat& sformat);

    StreamFormat getStreamFormatFromVideoMode(const cs::VideoMode& videomode);

    Frame copyFrame(const Frame& frame) noexcept;
}