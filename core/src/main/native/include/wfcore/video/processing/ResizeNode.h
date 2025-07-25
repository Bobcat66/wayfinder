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

namespace wf {
    template <CVImage T>
    class ResizeNode : public CVProcessNode<T> {
    public:
        ResizeNode(int interpolater_, int outWidth_, int outHeight_);
        ResizeNode(int outWidth_, int outHeight_);
        void updateBuffers() override;
        void process() noexcept override;
    private:
        int interpolater;
        cv::Size outsize;
    };
}