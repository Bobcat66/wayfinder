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


#include "wfcore/video/video_utils.h"
#include <opencv2/core.hpp>
#include <stdexcept>

namespace wf {
    cv::Mat generateEmptyFrameBuf(FrameFormat format) {
        return cv::Mat(format.height,format.width,getCVTypeFromEncoding(format.encoding));
    };

    cs::VideoMode getVideoModeFromStreamFormat(const StreamFormat& sformat) {
        return {
            getPixelFormatFromEncoding(sformat.frameFormat.encoding),
            sformat.frameFormat.width,
            sformat.frameFormat.height,
            sformat.fps
        };
    }

    StreamFormat getStreamFormatFromVideoMode(const cs::VideoMode& videomode) {
        return {
            videomode.fps,
            {
                getEncodingFromPixelFormat(
                    static_cast<cs::VideoMode::PixelFormat>(videomode.pixelFormat)
                ),
                videomode.height,
                videomode.width
            }
        };
    }

    void sparseResize(const std::vector<cv::Point2d>& input, std::vector<cv::Point2d>& output, double fx, double fy) noexcept {
        output.resize(input.size());
        for (size_t i = 0; i < input.size(); ++i) {
            const auto& input_point = input[i];
            output[i] = {input_point.x * fx,input_point.y * fy};
        }
    }

    void sparseResize(const std::vector<cv::Point2f>& input, std::vector<cv::Point2f>& output, float fx, float fy) noexcept {
        output.resize(input.size());
        for (size_t i = 0; i < input.size(); ++i) {
            const auto& input_point = input[i];
            output[i] = {input_point.x * fx,input_point.y * fy};
        }
    }

    void sparseShift(const std::vector<cv::Point2d>& input, std::vector<cv::Point2d>& output, double dx, double dy) noexcept {
        output.resize(input.size());
        for (size_t i = 0; i < input.size(); ++i) {
            const auto& input_point = input[i];
            output[i] = {input_point.x + dx,input_point.y + dy};
        }
    }

    void sparseShift(const std::vector<cv::Point2f>& input, std::vector<cv::Point2f>& output, float dx, float dy) noexcept {
        output.resize(input.size());
        for (size_t i = 0; i < input.size(); ++i) {
            const auto& input_point = input[i];
            output[i] = {input_point.x + dx,input_point.y + dy};
        }
    }
}