#pragma once

#include "wfcore/video/processing/CVProcessNode.h"

namespace wf {
    template <CVImage T>
    class RotateNode : CVProcessNode<T> {
    public:
        RotateNode(int rotation_);
        void setInpad(const T& inpad) override;
        void process() noexcept override;
    private:
        int rotation;
    };
}