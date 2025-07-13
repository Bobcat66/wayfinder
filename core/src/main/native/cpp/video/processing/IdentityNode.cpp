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

#include "wfcore/video/processing/IdentityNode.h"

namespace wf {
    template <CVImage T>
    void IdentityNode<T>::updateBuffers() {
        this->outpad = *(this->inpad);
        this->outcoding = *(this->incoding);
    }

    template <CVImage T>
    void IdentityNode<T>::process() noexcept override {
        this->outpad = *(this->inpad);
    }

    template class IdentityNode<cv::Mat>;
    template class IdentityNode<cv::UMat>;
}