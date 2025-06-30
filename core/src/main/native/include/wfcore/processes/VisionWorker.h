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

#include "wfcore/pipeline/Pipeline.h"
#include "wfcore/video/FrameProvider.h"
#include "wfcore/video/processing/CVProcessPipe.h"

namespace wf {

    typedef std::function<void(const PipelineResult&)> ResultConsumer;
    typedef std::function<void(const Frame&)> FrameConsumer;

    class VisionWorker {
    public:
        VisionWorker(
            std::string name_,
            FrameProvider& frameProvider_, 
            CVProcessPipe<cv::Mat>& preprocessor_,
            Pipeline& pipeline_, 
            ResultConsumer resultConsumer_,
            FrameConsumer frameConsumer_
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
        CVProcessPipe<cv::Mat>& preprocesser;
        Pipeline& pipeline;
        FrameProvider& frameProvider;
        ResultConsumer resultConsumer;
        FrameConsumer frameConsumer;
    };
}