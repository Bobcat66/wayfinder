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

#include "wfcore/video/processing/LetterboxNode.h"
#include "wfcore/common/wfexcept.h"
#include <array>
#include <algorithm>

#define SOURCE_WIDTH this->inpad->cols
#define SOURCE_HEIGHT this->inpad->rows
#define SOURCE_ENCODING *(this->incoding)
#define SOURCE_CVTYPE this->inpad->type()

namespace wf {

    constexpr std::array<ImageEncoding,3> validEncodings = {
        ImageEncoding::BGR24,
        ImageEncoding::RGB24,
        ImageEncoding::Y8
    };

    template <CVImage T>
    LetterboxNode<T>::LetterboxNode(
        int targetWidth_,
        int targetHeight_,
        cv::Scalar fillColor_,
        int interpolater_
    )
    : targetWidth(targetWidth_)
    , targetHeight(targetHeight_)
    , fillColor(fillColor_)
    , interpolater(interpolater_) {}

    template <CVImage T>
    void LetterboxNode<T>::updateBuffers() {
        if (std::find(validEncodings.begin(), validEncodings.end(), SOURCE_ENCODING) == validEncodings.end()) {
            throw invalid_image_encoding("Attempted to use an invalid encoding for a LetterboxNode. LetterboxNodes only support BGR24, RGB24, and Y8 as of now");
        }
        this->outpad = T(
            targetHeight,
            targetWidth,
            SOURCE_CVTYPE
        );
        this->outcoding = SOURCE_ENCODING;
        scale = std::min(
            static_cast<double>(targetWidth)/SOURCE_WIDTH,
            static_cast<double>(targetHeight)/SOURCE_HEIGHT
        );
        resizedWidth = static_cast<int>(SOURCE_WIDTH * scale);
        resizedHeight = static_cast<int>(SOURCE_HEIGHT * scale);
        leftPadding = (targetWidth - resizedWidth)/2;
        rightPadding = targetWidth - resizedWidth - leftPadding;
        topPadding = (targetHeight - resizedHeight)/2;
        bottomPadding = targetHeight - resizedHeight - topPadding;

        resizedImageBuffer.create(
            resizedHeight,
            resizedWidth,
            SOURCE_CVTYPE
        );
    }

    template <CVImage T>
    void LetterboxNode<T>::process() noexcept {
        cv::resize(*(this->inpad),resizedImageBuffer,{},scale,scale,interpolater);
        cv::copyMakeBorder(
            resizedImageBuffer,
            this->outpad,
            topPadding,
            bottomPadding,
            leftPadding,
            rightPadding,
            cv::BORDER_CONSTANT,
            fillColor
        );
    }
}

#undef SOURCE_WIDTH
#undef SOURCE_HEIGHT
#undef SOURCE_ENCODING
#undef SOURCE_CVTYPE