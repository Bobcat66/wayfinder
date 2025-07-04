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
#include "wfcore/video/video_utils.h"

namespace wf {
    template <CVImage T>
    class CVProcessPipe {
    public:
        template <std::derived_from<CVProcessNode<T>>... Nodes>
        CVProcessPipe(FrameFormat inputFormat, Nodes&... nodes) {
            inpad = generateEmptyCVImg<T>(inputFormat);
            (this->nodes.push_back(std::ref(nodes)), ...);
            const T* pad = &inpad;
            for (auto& node : this->nodes) {
                node.get().setInpad(pad);
                pad = &(node.get().getOutpad());
            }
            this->outpad = pad;
            this->outformat = getFormat(*outpad);
        }
        /*
        Processes an image without any internal allocations. NOTE: OUTPAD WILL BE CONNECTED TO INTERNAL BUFFERS AFTER THIS OPERATION!
        DO NOT REUSE THE CVProcessPipe until you are done with any additional processing you want to do on this frame, or make a deepcopy.
        in MUST be in the correct FrameFormat (height, width, channels, type). Passing a mat of the wrong FrameFormat results in UNDEFINED BEHAVIOR!!!
        */
        inline void processDirect(const T& in, T& out) noexcept {
            inpad = in;
            process();
            out = *outpad;
        }

        void setInputFormat(FrameFormat inputFormat) {
            inpad = generateEmptyCVImg<T>(inputFormat);
            for (auto& node : this->nodes) {
                node.get().updateBuffers();
            }
            this->outformat = getFormat(*(this->outpad));
        }
        inline const FrameFormat& getInputFormat() {return this->informat;}

        [[nodiscard]]
        inline Frame processFrame(const Frame& in) noexcept {
            Frame out(in.captimeMicros,this->outformat,{});
            in.data.copyTo(inpad);
            process();
            outpad->copyTo(out.data);
            return out;
        }

    private:
        void process() noexcept {
            for (auto& node : nodes) {
                node.get().process();
            }
        }
        std::vector<std::reference_wrapper<CVProcessNode<T>>> nodes;
        FrameFormat informat;
        FrameFormat outformat;
        T inpad;
        const T* outpad;
    };
}