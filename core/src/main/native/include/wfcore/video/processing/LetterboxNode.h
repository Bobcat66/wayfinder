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

#include "wfcore/video/processing/CVProcessNode.h"
#include "wfcore/video/video_types.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace wf {
    template <CVImage T>
    class LetterboxNode : public CVProcessNode<T> {
    public:
        LetterboxNode(
            int targetWidth_,
            int tagetHeight_,
            cv::Scalar fillColor_ = cv::Scalar(114,114,114),
            int interpolater_ = cv::INTER_LINEAR
        );
        void updateBuffers() override;
        void process() noexcept override;
    private:
        int targetWidth;
        int targetHeight;
        double scale;
        
        int resizedWidth;
        int resizedHeight;

        int leftPadding;
        int topPadding;
        int rightPadding;
        int bottomPadding;
        cv::Scalar fillColor;
        int interpolater;
        T resizedImageBuffer; // Temporary buffer to store resized image before applying the border
    };
}