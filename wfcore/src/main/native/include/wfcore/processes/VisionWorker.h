#pragma once

#include <opencv2/core.hpp>

#include <thread>
#include <atomic>

#include "wfcore/pipeline/Pipeline.h"
#include "wfcore/video/FrameProvider.h"

namespace wf {
    class VisionProcess {
    public:
        VisionProcess()
        void start();
        void stop();
        private:
        void run() noexcept;
        cv::Mat inputBuffer;
        cv::Mat outputBuffer;
        std::thread thread;
        std::atomic_bool running;
        Pipeline pipeline;
        FrameProvider frameProvider;
        void (*resultConsumer)(PipelineResult result)
    };
}