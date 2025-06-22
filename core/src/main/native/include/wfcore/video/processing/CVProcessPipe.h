#pragma once

#include <vector>
#include "wfcore/video/video_types.h"
#include "wfcore/video/processing/CVProcessNode.h"

namespace wf {
    template <CVImage T>
    class CVProcessPipe {
    public:
        CVProcessPipe(CVProcessNode<T>... nodes);
        void process(const T& in, T& out);
        Frame processFrame(const Frame& in);
    private:
        std::vector<CVProcessNode<T>> nodes;
        const T inpad;
        T outpad;
        void (*unwrap)(const Frame& in,T& out); // Unwraps a frame
        void (*wrap)(const T& in, Frame& out); // Wraps a buffer into a frame
    }
}