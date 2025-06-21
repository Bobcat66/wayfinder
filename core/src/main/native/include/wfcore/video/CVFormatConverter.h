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
    class CVFormatConverter {
    public:
        CVFormatConverter(FrameFormat in, FrameFormat out);
        ~CVFormatConverter() = default;
        void convert(cv::Mat& in, cv::Mat& out) noexcept; // This method mutates a cv matrix in-place
        const FrameFormat& getInputFormat() const {return inFormat;}
        const FrameFormat& getOutputFormat() const {return outFormat;}
    private:
        FrameFormat inFormat;
        FrameFormat outFormat;
        FrameFormat tmpFormat;
        double xscalefactor;
        double yscalefactor;
        cv::Mat buffer;
        void (*colorConverter)(cv::Mat& in,cv::Mat& out);
    };
}