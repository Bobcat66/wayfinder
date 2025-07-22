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
#include "wfcore/common/logging.h"
#include "wfcore/video/video_types.h"

namespace impl {
    [[ nodiscard ]]
    static inline bool validateFrame(const cv::Mat& frame,const wf::FrameMetadata& meta) noexcept {
        return (frame.rows == meta.format.height) 
            && (frame.cols == meta.format.width) 
            && (frame.type() == wf::getCVTypeFromEncoding(meta.format.encoding));
    }
}

namespace wf {

    using enum VisionWorkerStatus;
    
    VisionWorker::VisionWorker(
        std::string name_,
        FrameProvider& frameProvider_,
        CVProcessPipe<cv::Mat> preprocesser_, 
        std::unique_ptr<Pipeline> pipeline_,
        std::unique_ptr<PipelineOutputConsumer> outputConsumer_
    )
    : name(std::move(name_))
    , preprocesser(std::move(preprocesser_))
    , frameProvider(frameProvider_)
    , pipeline(std::move(pipeline_))
    , outputConsumer(std::move(outputConsumer_)) 
    , ConcurrentLoggedStatusfulObject<VisionWorkerStatus,VisionWorkerStatus::Ok>(name,LogGroup::General) {
        auto& rawformat = frameProvider.getStreamFormat().frameFormat;
        rawFrameBuffer.create(
            rawformat.width,
            rawformat.height,
            getCVTypeFromEncoding(
                rawformat.encoding
            )
        );
        running = false;
    }

    void VisionWorker::start() {
        running = true;
        thread = std::thread(&VisionWorker::run,this);
    }

    void VisionWorker::stop() {
        if (running) {
            running = false;
            thread.join();
        }
    }

    // TODO: Add more robust error handling
    void VisionWorker::run() noexcept {
        while (running.load()) {
            std::lock_guard<std::mutex> lock(pipeGuard);
            auto rawmeta = frameProvider.getFrame(rawFrameBuffer);
            if (!(frameProvider.ok())) {
                const auto errmsg(frameProvider.getError().value());
                this->reportError(ProviderError,errmsg);
                continue;
            }
            // The expected frame formats are negotiated during configuration, all we need during runtime is a simple sanity check
            if (!impl::validateFrame(rawFrameBuffer,rawmeta)) {
                this->reportError(InvalidFrame,"Invalid frame received from frame provider");
                continue;
            }
            auto ppmeta = preprocesser.processFrame(rawFrameBuffer,ppFrameBuffer,rawmeta);
            if (!impl::validateFrame(ppFrameBuffer,ppmeta)) {
                this->reportError(InvalidFrame,"Invalid frame received from preprocesser");
                continue;
            }
            auto res = pipeline->process(ppFrameBuffer,ppmeta);
            if (!(pipeline->ok())) {
                // TODO: More robust error handling
                const auto errmsg(pipeline->getError().value());
                this->reportError(
                    VisionWorkerStatus::PipelineError,
                    "Error in pipeline: {}", errmsg
                );
                continue;
            }
            outputConsumer->accept(ppFrameBuffer,ppmeta,res);
            this->reportOk();
        }
    }
}