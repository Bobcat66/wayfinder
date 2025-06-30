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

#include "wfcore/processes/VisionWorker.h"
#include "wfcore/video/video_utils.h"

namespace wf {
    VisionWorker::VisionWorker(
        std::string name_,
        FrameProvider& frameProvider_,
        CVProcessPipe<cv::Mat>& preprocesser_, 
        Pipeline& pipeline_,
        ResultConsumer resultConsumer_,
        FrameConsumer frameConsumer_
    )
    : name(std::move(name_))
    , preprocesser(preprocesser_)
    , frameProvider(frameProvider_)
    , pipeline(pipeline_)
    , resultConsumer(std::move(resultConsumer_)) 
    , frameConsumer(std::move(frameConsumer_)) {
        running = false;
    }

    void VisionWorker::start() {
        running = true;
        thread = std::thread(&VisionWorker::run,this);
    }

    void VisionWorker::stop() {
        if(running){
            running = false;
            thread.join();
        }
    }

    void VisionWorker::run() noexcept {
        while (running.load()) {
            auto frame = frameProvider.getFrame();
            auto ppframe = preprocesser.processFrame(frame);
            auto res = pipeline.process(ppframe);
            resultConsumer(res);
        }
    }
}