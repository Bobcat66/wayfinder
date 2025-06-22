#pragma once

#include "wfcore/video/processing/CVProcessNode.h"
#include "wfcore/video/video_types.h"
#include <opencv2/core.hpp>

namespace wf {
    template <CVImage T>
    class ResizeNode : CVProcessNode<T> {
    public:
        ResizeNode(int interpolater_, int outWidth_, int outHeight_);
        void setInpad(const T& inpad) override;
        void process() noexcept override;
    private:
        int interpolater;
        cv::Size outsize;
    };
}