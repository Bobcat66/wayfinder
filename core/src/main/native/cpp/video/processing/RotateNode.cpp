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
        this->outpad = T(
            this->outsize.width,
            this->outsize.height,
            this->inpad->type()
        );
    }

    template <CVImage T>
    void RotateNode<T>::process() noexcept {
        cv::rotate(*(this->inpad),this->outpad,this->rotation);
    }
}