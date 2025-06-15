#pragma once

#include <opencv2/core/mat.hpp>
#include <cstdint>

namespace wf {

    enum ColorSpace {
        COLOR, // CV_8UC3, cscore kBGR
        GRAY, // CV_8UC1, cscore kGray
        DEPTH, // CV_16UC1, cscore kY16
        UNKNOWN
    };

    // Frameformat only supports a boolean for color space because our code only ever interacts with two colorspaces: BGR for color images and GRAY8 for grayscale
    struct FrameFormat {
        ColorSpace colorspace;
        int rows;
        int cols;
    };

    struct StreamFormat {
        int fps;
        FrameFormat frameFormat;
    };

    struct Frame {
        uint64_t captimeMicros;
        FrameFormat format;
        cv::Mat data;
    };
}