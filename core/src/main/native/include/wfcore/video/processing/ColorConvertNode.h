#pragma once

#include "wfcore/video/processing/CVProcessNode.h"
#include "wfcore/video/video_types.h"

namespace wf {
    template <CVImage T>
    class ColorConvertNode : CVProcessNode<T> {
    public:
        ColorConvertNode(const T& inpad, ColorSpace inspace, ColorSpace outspace);
        void process() noexcept override;
    private:
        void (*colorConverter)(const T& in,T& out);
    };
}