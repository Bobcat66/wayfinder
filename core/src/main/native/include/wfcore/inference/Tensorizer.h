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

#include "wfcore/inference/inference_configs.h"
#include <opencv2/core.hpp>
#include <vector>

// Tensorization is an additionak preprocessing step that converts an input frame into a tensor format suitable for inference.
// As channel reordering (in the form of color space conversion), cropping, and resizing are assumed to be done outside of this class,
// this class focuses on the final steps of tensorization, which include centering, normalization, and standardization
namespace wf {

    class Tensorizer {
    public:
        Tensorizer() = default;
        ~Tensorizer() = default;
        void setTensorParameters(TensorParameters params);
        void tensorize(const cv::Mat& input, float* tensorBuffer) const noexcept;
        const TensorParameters& getTensorParameters() const noexcept { return params; }
    private:
        TensorParameters params; // Parameters for how the input frame should be tensorized
        cv::Mat temp; // Temporary buffer to hold input frame
        std::vector<cv::Mat> channels; // Temporary buffer to hold channels of the input frame. This is used as tensors store image data in planar format, where each channel is stored contiguously in memory, while OpenCV mats are stored in interleaved format, where each pixel contains all channels. This is used to convert the input frame into the correct tensor format
    };
}