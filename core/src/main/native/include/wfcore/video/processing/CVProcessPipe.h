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
            unwrap(in,inpad);
            process();
            return wrap(outpad);
        }

    private:
        void process() noexcept;
        std::vector<CVProcessNode<T>> nodes;
        FrameFormat informat;
        FrameFormat outformat;
        const T inpad;
        T outpad;
        void (*unwrap)(const Frame& in,T& out); // Unwraps a frame
        Frame (*wrap)(const T& in); // Wraps a buffer into a frame
    };
}