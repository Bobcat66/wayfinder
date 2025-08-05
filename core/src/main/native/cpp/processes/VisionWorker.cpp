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
#include <pthread.h>

namespace impl {
    [[ nodiscard ]]
    static inline bool validateFrame(const cv::Mat& frame,const wf::FrameMetadata& meta) noexcept {
        return (frame.rows == meta.format.height) 
            && (frame.cols == meta.format.width) 
            && (frame.type() == wf::getCVTypeFromEncoding(meta.format.encoding));
    }

    const char* setThreadName(const std::string& name) {
        // pthread_setname_np limits names to 16 characters including null terminator
        pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());
        return name.substr(0, 15).c_str();
    }
}

namespace wf {

    using enum WFStatus;
    
    VisionWorker::VisionWorker(
        std::string name_,
        std::shared_ptr<FrameProvider> frameProvider_,
        CVProcessPipe<cv::Mat> preprocesser_, 
        std::unique_ptr<Pipeline> pipeline_,
        std::unique_ptr<PipelineOutputConsumer> outputConsumer_
    )
    : name(std::move(name_))
    , preprocesser(std::move(preprocesser_))
    , frameProvider(frameProvider_)
    , pipeline(std::move(pipeline_))
    , outputConsumer(std::move(outputConsumer_)) 
    , WFConcurrentLoggedStatusfulObject(name,LogGroup::General) {
        auto sfres = frameProvider->getStreamFormat();
        if (!sfres)
            throw wf_result_error(sfres);
        auto rawformat = sfres.value().frameFormat;
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
        if (running.load()) return;
        running.store(true);
        thread = std::jthread([this](std::stop_token stoken){
            this->run(stoken);
        });
    }

    void VisionWorker::stop() {
        if (thread.joinable()) {
            thread.request_stop(); // cooperative stop
            thread.join();         // waits for thread to exit
        }
        running.store(false);
    }

    // TODO: Add more robust error handling
    void VisionWorker::run(std::stop_token stoken) noexcept {
        threadName = impl::setThreadName(name);
        while (!stoken.stop_requested()) {
            if (!ok()) continue;
            auto rawmeta = frameProvider->getFrame(rawFrameBuffer);
            if (!rawmeta) {
                //const auto errmsg(frameProvider.getError().value());
                this->reportError(rawmeta.status);
                continue;
            }
            // The expected frame formats are negotiated during configuration, all we need during runtime is a simple sanity check
            if (!impl::validateFrame(rawFrameBuffer,rawmeta)) {
                this->reportError(PIPELINE_BAD_FRAME,"Bad frame received from source");
                continue;
            }
            auto ppmeta = preprocesser.processFrame(rawFrameBuffer,ppFrameBuffer,rawmeta);
            if (!impl::validateFrame(ppFrameBuffer,ppmeta)) {
                this->reportError(PIPELINE_BAD_FRAME,"Bad frame received from preprocesser");
                continue;
            }
            auto res = pipeline->process(ppFrameBuffer,ppmeta);
            if (!res) {
                this->reportError(res);
                continue;
            }
            outputConsumer->accept(ppFrameBuffer,ppmeta,res.value());
        }
        running.store(false);
    }
}