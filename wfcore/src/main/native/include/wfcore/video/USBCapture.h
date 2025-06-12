#pragma once

#include <wfcore/video/code.h>
#include <opencv2/core/mat.hpp>

#include <cstdint>
#include <string>

namespace wf::video {

    struct USBCaptureConfig {
        std::string dev_name; // example: /dev/video0 on linux
        Compression compression;
        std::string codec; // codec suggestion. The video capture backend may use a different codec if the one specified isn't present
        bool grayscale;
    };

    class USBCapture {
        public:
        USBCapture(const USBCaptureConfig& config);
        ~USBCapture();
        // Grab latest frame from gstreamer, timing out after timeout ms. Returns the timestamp of the frame capture in microseconds, or 0 if the operation was unsuccessful
        uint64_t grabFrame(cv::Mat& frame,int timeout) const;
        private:
        void *pipeline, *source, *decode, *convert, *appsink;
    };

}