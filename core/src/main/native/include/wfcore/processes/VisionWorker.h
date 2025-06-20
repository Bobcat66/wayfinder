#pragma once

#include <opencv2/core.hpp>

#include <thread>
#include <atomic>
#include <functional>

#include "wfcore/pipeline/Pipeline.h"
#include "wfcore/video/FrameProvider.h"

namespace wf {

    typedef std::function<void(const PipelineResult&)> ResultConsumer;

    class VisionWorker {
    public:
        VisionWorker(
            Pipeline& pipeline_, 
            FrameProvider& frameProvider_, 
            ResultConsumer resultConsumer_
        );
        void start();
        void stop();
        private:
        void run() noexcept;
        cv::Mat inputBuffer;
        cv::Mat outputBuffer;
        std::thread thread;
        std::atomic_bool running;
        Pipeline& pipeline;
        FrameProvider& frameProvider;
        ResultConsumer resultConsumer;
    };
}