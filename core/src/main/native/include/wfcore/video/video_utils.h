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

#pragma once

#include "wfcore/video/video_types.h"

namespace wf {
    cv::Mat generateEmptyFrameBuf(FrameFormat format);

    template <CVImage T>
    FrameFormat getFormat(const T& cvimg) {
        ColorSpace cspace;
        switch (cvimg.type()) {
            case CV_8UC3:
                cspace = ColorSpace::COLOR;
                break;
            case CV_8UC1:
                cspace = ColorSpace::GRAY;
                break;
            case CV_16UC1:
                cspace = ColorSpace::DEPTH;
                break;
            default:
                cspace = ColorSpace::UNKNOWN;
        }
        return {cspace,cvimg.rows,cvimg.cols};
    }
}