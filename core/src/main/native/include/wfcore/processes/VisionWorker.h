/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of Wayfinder.
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

#include "wfcore/pipeline/Pipeline.h"
#include "wfcore/video/FrameProvider.h"
#include "wfcore/video/processing/CVProcessPipe.h"

namespace wf {

    typedef std::function<void(const PipelineResult&)> ResultConsumer;

    class VisionWorker {
    public:
        VisionWorker(
            FrameProvider& frameProvider_, 
            CVProcessPipe<cv::Mat>& preprocessor_,
            Pipeline& pipeline_, 
            ResultConsumer resultConsumer_
        );
        void start();
        void stop();
        private:
        void run() noexcept;
        std::thread thread;
        std::atomic_bool running;
        CVProcessPipe<cv::Mat>& preprocesser;
        Pipeline& pipeline;
        FrameProvider& frameProvider;
        ResultConsumer resultConsumer;
    };
}