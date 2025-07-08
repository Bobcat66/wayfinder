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

#include "wfcore/video/processing/RotateNode.h"
#include <opencv2/imgproc.hpp>

namespace wf {

    template <CVImage T>
    RotateNode<T>::RotateNode(int rotation_) : rotation(rotation) {}

    template <CVImage T>
    void RotateNode<T>::updateBuffers() {
        switch (this->rotation) {
            case cv::ROTATE_90_CLOCKWISE:
            case cv::ROTATE_90_COUNTERCLOCKWISE:
                this->outpad = T(this->inpad->cols,this->inpad->rows,this->inpad->type());
                break;
            default:
                this->outpad = T(this->inpad->rows,this->inpad->cols,this->inpad->type());
        }
        this->outcoding = *(this->incoding);
    }

    template <CVImage T>
    void RotateNode<T>::process() noexcept {
        cv::rotate(*(this->inpad),this->outpad,this->rotation);
    }

    template class RotateNode<cv::Mat>;
    template class RotateNode<cv::UMat>;
}