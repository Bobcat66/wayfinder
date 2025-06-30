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


#include "wfcore/inference/Tensorizer.h"

#include <cassert>
#include <cstring>

namespace wf {
    void Tensorizer::setTensorParameters(const TensorParameters& params) {
        this->params = params;
        temp.create(params.height, params.width, CV_32FC(params.channels));
        channels.clear();
        channels.reserve(params.channels);
        for (int i = 0; i < params.channels; ++i) {
            channels.emplace_back(params.height, params.width, CV_32FC1);
        }
    }
    void Tensorizer::tensorize(const cv::Mat& input, float* output) const noexcept {
        input.convertTo(temp, CV_32F, params.scale);
        cv::divide(temp, params.stds, temp);
        cv::subtract(temp, params.means, temp);
        
        if (!params.interleaved) {
            cv::split(temp, channels);
            for (int c = 0; c < params.channels; ++c) {
                for (int h = 0; h < params.height; ++h) {
                    const float* srcrow = channels[c].ptr<float>(h);
                    float* dstrow = output + (c * params.height * params.width) + h * params.width;
                    std::memcpy(dstrow, srcrow, params.width * sizeof(float));
                }
            }        
        } else {
            for (int h = 0; h < params.height; ++h) {
                const float* srcrow = temp.ptr<float>(h);
                float* dstrow = output + h * params.width * params.channels;
                std::memcpy(dstrow, srcrow, params.width * params.channels * sizeof(float));
            }
        }
    }
}