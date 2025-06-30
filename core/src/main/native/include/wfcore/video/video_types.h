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

    enum ColorSpace {
        COLOR, // CV_8UC3, cscore kBGR
        GRAY, // CV_8UC1, cscore kGray
        DEPTH, // CV_16UC1, cscore kY16
        RGB, // CV_8UC3, cscore kRGB
        UNKNOWN
    };

    // Frameformat only supports a boolean for color space because our code only ever interacts with two colorspaces: BGR for color images and GRAY8 for grayscale
    struct FrameFormat {
        ColorSpace colorspace;
        int rows;
        int cols;
    };

    struct StreamFormat {
        int fps;
        FrameFormat frameFormat;
    };

    struct Frame {
        uint64_t captimeMicros;
        FrameFormat format;
        cv::Mat data;
    };
}