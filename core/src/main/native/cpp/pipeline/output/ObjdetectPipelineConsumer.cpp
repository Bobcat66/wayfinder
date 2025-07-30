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

#include "wfcore/pipeline/output/ObjdetectPipelineConsumer.h"
#include <format>
#include "wfcore/pipeline/annotations.h"
#include "wfcore/common/wfexcept.h"

//TODO refactor to use status.h
namespace wf {
    ObjdetectPipelineConsumer::ObjdetectPipelineConsumer(
        std::string pipelineName_, std::string camLabel_, 
        FrameFormat inputFormat_, 
        int rawPort, int processedPort,
        StreamFormat streamFormat,
        std::weak_ptr<NTDataPublisher> ntpub_
    ) 
    : pipelineName(pipelineName_), camLabel(camLabel_)
    , inputFormat(inputFormat_)
    , rawServer(streamFormat,std::format("{}_raw",pipelineName),rawPort)
    , processedServer(streamFormat,std::format("{}_processed",pipelineName),processedPort)
    , ntpub(std::move(ntpub_))
    {
        if (!(streamFormat.frameFormat.encoding == ImageEncoding::BGR24)) {
            throw invalid_image_encoding("As of now, output pixel format for OpenCV stream MUST be BGR24");
        }
        std::vector<std::unique_ptr<CVProcessNode<cv::Mat>>> nodes;
        if (streamFormat.frameFormat == inputFormat) {
            nodes.emplace_back(std::make_unique<IdentityNode<cv::Mat>>());
        } else {
            if (streamFormat.frameFormat.width != inputFormat.width || streamFormat.frameFormat.height != inputFormat.height) {
                nodes.emplace_back(
                    std::make_unique<ResizeNode<cv::Mat>>(
                        cv::INTER_LINEAR,
                        streamFormat.frameFormat.width,
                        streamFormat.frameFormat.height
                    )
                );
            }

            if (streamFormat.frameFormat.encoding != ImageEncoding::BGR24){
                nodes.emplace_back(
                    std::make_unique<ColorConvertNode<cv::Mat>>(
                        ImageEncoding::BGR24
                    )
                );
            }
        }
        prePostprocessor = std::make_unique<CVProcessPipe<cv::Mat>>(inputFormat,std::move(nodes));
    }

    bool ObjdetectPipelineConsumer::accept(cv::Mat& data, FrameMetadata meta, PipelineResult& result) noexcept {
        if (auto shared = ntpub.lock()) {
            shared->publishPipelineResult(result);
        } else {
            return false;
        }
        if (this->streamingEnabled_) {
            // TODO: characterize
            auto rmeta = prePostprocessor->processFrame(data,pp_rawbuf,meta);
            pp_rawbuf.copyTo(pp_procbuf);
            drawCamLabel(pp_rawbuf,std::format("{} RAW",camLabel));
            rawServer.acceptFrame(pp_rawbuf,rmeta);
            drawCamLabel(pp_procbuf,std::format("{} PROCESSED",camLabel));
            for (const auto& detection : result.objectDetections) {
                drawBbox(pp_procbuf,detection);
            }
            processedServer.acceptFrame(pp_procbuf,rmeta);
        }
        return true;
    }
}