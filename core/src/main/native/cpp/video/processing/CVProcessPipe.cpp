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

#include "wfcore/video/processing/CVProcessPipe.h"
#include <stdexcept>
#include <type_traits>

namespace wf {

    template <CVImage T>
    template <std::same_as<CVProcessNode<T>>... Nodes>
    CVProcessPipe<T>::CVProcessPipe(Nodes&... nodes) {
        (this->nodes.push_back(std::ref(nodes)), ...);
        T* pad = &inpad;
        for (auto node : this->nodes) {
            node.setInpad(pad);
            inpad = &(node.getOutpad());
        }
        this->outpad = *pad;

        if constexpr (std::is_same_v<T,cv::Mat>()) {
            wrap = [](const T& in,Frame& out) {
                out.data = in;
            };
        } else if constexpr (std::is_same_v<T,cv::UMat>()){
            wrap = [](const T& in,Frame& out) {
                out.data = in.getMat();
            };
        }
    }

    template <CVImage T>
    void CVProcessPipe<T>::process() noexcept {
        for (auto& node : nodes) {
            node.process();
        }
    }

    template class CVProcessPipe<cv::Mat>;
    template class CVProcessPipe<cv::UMat>;
}