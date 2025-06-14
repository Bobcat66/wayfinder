#pragma once

#include <opencv2/core/mat.hpp>
#include <cstdint>

namespace wf {

    struct FrameFormat {
        bool color;
        int rows;
        int cols;
    };

    struct Frame {
        uint64_t captimeMicros;
        FrameFormat format;
        cv::Mat data;
    };
}