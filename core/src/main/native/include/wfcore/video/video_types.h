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

#include <opencv2/core/mat.hpp>
#include <cstdint>
#include <concepts>

namespace wf {

    template <typename T>
    concept CVImage = std::same_as<T, cv::Mat> || std::same_as<T, cv::UMat>;

    enum class ImageEncoding : uint8_t {
        BGR24, // Blue-Green-Red 24 bit, default color encoding
        RGB24,
        RGB565, 
        Y8, // Grayscale 8 bit, default gray encoding
        Y16, // Grayscale 16 bit, default depth & IR encoding
        YUYV,
        UYVY,
        RGBA,
        BGRA,
        MJPEG,
        UNKNOWN
    };

    // TODO: Change order to be (width,height,encoding)
    struct FrameFormat {
        ImageEncoding encoding;
        int rows;
        int cols;

        bool operator==(const FrameFormat& other) const {
            return encoding == other.encoding
                && rows == other.rows 
                && cols == other.cols;
        }
    };

    struct StreamFormat {
        int fps;
        FrameFormat frameFormat;
        
        bool operator==(const StreamFormat& other) const {
            return fps == other.fps && frameFormat == other.frameFormat;
        }
    };

    struct FrameMetadata {
        uint64_t micros;
        FrameFormat format;

        constexpr bool err() const { return !(static_cast<bool>(micros)); }
    };
}