#pragma once

#include "wfcore/video/processing/CVProcessNode.h"
#include "wfcore/video/video_types.h"
#include <opencv2/core.hpp>

namespace wf {
    template <CVImage T>
    class ResizeNode : CVProcessNode<T> {
    public:
        ResizeNode(const T& inpad_, int interpolater_, int outWidth_, int outHeight_);
        void process() noexcept override;
    private:
        int interpolater;
        cv::Size outsize;
    };
}