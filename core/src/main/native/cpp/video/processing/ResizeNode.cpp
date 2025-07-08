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

#include "wfcore/video/processing/ResizeNode.h"
#include <opencv2/imgproc.hpp>
namespace wf {

    template <CVImage T>
    ResizeNode<T>::ResizeNode(int interpolator_, int outX_, int outY_) 
    : outsize(outX_,outY_)
    , interpolater(interpolator_) {}

    template <CVImage T>
    void ResizeNode<T>::updateBuffers() {
        this->outpad = T(
            this->outsize.width,
            this->outsize.height,
            this->inpad->type()
        );
        this->outcoding = *(this->incoding);
    }

    template <CVImage T>
    void ResizeNode<T>::process() noexcept {
        cv::resize(
            *(this->inpad),
            this->outpad,
            this->outsize,
            this->interpolater
        );
    }

    template class ResizeNode<cv::Mat>;
    template class ResizeNode<cv::UMat>;
}