#pragma once

#include "wfcore/video/video_types.h"

namespace wf {
    class CVFormatConverter {
    public:
        CVFormatConverter(FrameFormat in, FrameFormat out);
        ~CVFormatConverter() = default;
        void convert(cv::Mat& in, cv::Mat& out) noexcept; // This method mutates a cv matrix in-place
        const FrameFormat& getInputFormat() const {return inFormat;}
        const FrameFormat& getOutputFormat() const {return outFormat;}
    private:
        FrameFormat inFormat;
        FrameFormat outFormat;
        FrameFormat tmpFormat;
        double xscalefactor;
        double yscalefactor;
        cv::Mat buffer;
        void (*colorConverter)(cv::Mat& in,cv::Mat& out);
    };
}