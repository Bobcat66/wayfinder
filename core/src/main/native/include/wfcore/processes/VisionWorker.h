/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2025 Jesse Kane
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#pragma once

#include <opencv2/core.hpp>

#include <thread>
#include <atomic>
#include <functional>
#include <string>

#include "wfcore/common/logging.h"
#include "wfcore/common/status.h"
#include "wfcore/pipeline/Pipeline.h"
#include "wfcore/pipeline/output/PipelineOutputConsumer.h"
#include "wfcore/video/FrameProvider.h"
#include "wfcore/video/processing/CVProcessPipe.h"

#include <mutex>

namespace wf {

    class VisionWorker : public WFConcurrentLoggedStatusfulObject {
    public:
        VisionWorker(
            std::string name_,
            std::shared_ptr<FrameProvider> frameProvider_, 
            CVProcessPipe<cv::Mat> preprocessor_,
            std::unique_ptr<Pipeline> pipeline_,
            std::unique_ptr<PipelineOutputConsumer> outputConsumer_
        );
        void start();
        void stop();
        const std::string& getName() const noexcept { return name; }
        const bool isRunning() const noexcept { return running.load(); }
    private:
        void run() noexcept;
        std::string name;
        std::thread thread;
        std::atomic_bool running;
        std::mutex pipeGuard;
        CVProcessPipe<cv::Mat> preprocesser;
        std::unique_ptr<Pipeline> pipeline;
        std::unique_ptr<PipelineOutputConsumer> outputConsumer;
        std::shared_ptr<FrameProvider> frameProvider;
        cv::Mat rawFrameBuffer;
        cv::Mat ppFrameBuffer;
    };
}