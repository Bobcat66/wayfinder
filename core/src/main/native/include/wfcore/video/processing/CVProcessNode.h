#pragma once

#include "wfcore/video/video_types.h"
#include "wfcore/video/video_utils.h"
namespace wf {
    template <CVImage T>
    class CVProcessNode {
    public:
        virtual ~CVProcessNode() = default;
        inline const T& getOutpad() const {return outpad;}
        inline FrameFormat getOutpadFormat() const {return getFormat(outpad);};
        virtual void process() noexcept = 0;
    protected:
        const T& inpad;
        T outpad;
        CVProcessNode(const T& inpad_) : inpad(inpad_) {}
    };
}