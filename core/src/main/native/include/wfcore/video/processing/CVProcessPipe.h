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

#include <vector>
#include "wfcore/video/video_types.h"
#include "wfcore/video/processing/CVProcessNode.h"

namespace wf {
    template <CVImage T>
    class CVProcessPipe {
    public:
        template <std::same_as<CVProcessNode<T>>... Nodes>
        CVProcessPipe(Nodes&... nodes);
        /*
        Processes an image with out any internal deep copies. NOTE: OUTPAD WILL BE CONNECTED TO INTERNAL BUFFERS AFTER THIS OPERATION!
        DO NOT REUSE THE CVProcessPipe until you are done with any additional processing you want to do on this frame, or make a deepcopy
        */
        void processDirect(const T& in, T& out) noexcept {
            inpad = in;
            process();
            out = outpad;
        }

        [[nodiscard]]
        inline Frame processFrame(const Frame& in) noexcept {
            Frame out(in.captimeMicros,this->outformat,{});
            unwrap(in,inpad);
            process();
            return out;
        }

    private:
        void process() noexcept;
        std::vector<CVProcessNode<T>> nodes;
        FrameFormat informat;
        FrameFormat outformat;
        T inpad;
        T outpad;
        void (*unwrap)(const Frame& in,T& out); // Unwraps a frame
        void (*wrap)(const T& in,Frame& out); // Wraps a buffer into a frame
    };
}