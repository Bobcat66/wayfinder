/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of Wayfinder.
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
        int cv_type;
        switch (format.colorspace) {
            case ColorSpace::COLOR:
                cv_type = CV_8UC3;
                break;
            case ColorSpace::GRAY:
                cv_type = CV_8UC1;
                break;
            case ColorSpace::DEPTH:
                cv_type = CV_16UC1;
                break;
            default:
                throw std::invalid_argument("unknown colorspace"); // TODO: switch to loggers once I get logging working
        }
        return cv::Mat(format.rows,format.cols,cv_type);
    };
}