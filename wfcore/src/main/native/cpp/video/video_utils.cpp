#include "wfcore/video/video_utils.h"
#include <opencv2/core.hpp>
#include <stdexcept>
namespace wf {
    cv::Mat generateEmptyFrameBuf(FrameFormat format) {
        int cv_type;
        switch (format.colorspace) {
            case ColorSpace::COLOR:
                cv_type = CV_8UC3;
                break;
            case ColorSpace::GRAY:
                cv_type = CV_8UC1;
                break;
            case ColorSpace::DEPTH:
                cv_type = CV_16UC1;
                break;
            default:
                throw std::invalid_argument("unknown colorspace"); // TODO: switch to loggers once I get logging working
        }
        return cv::Mat(format.rows,format.cols,cv_type);
    };
}