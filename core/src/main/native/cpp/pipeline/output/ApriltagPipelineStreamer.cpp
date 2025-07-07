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

#include "wfcore/pipeline/output/ApriltagPipelineStreamer.h"
#include <format>
#include <opencv2/imgproc.hpp>
#include "wfcore/network/NetworkTablesManager.h"
#include "wfcore/pipeline/annotations.h"
#include "wfcore/video/video_utils.h"

namespace wf {
    ApriltagPipelineStreamer::ApriltagPipelineStreamer(
        std::string pipelineName_, std::string camLabel_, 
        CameraIntrinsics intrinsics_, FrameFormat inputFormat_,
        int rawPort, int processedPort, 
        StreamFormat streamFormat, double tagSize_
    )
    : pipelineName(std::move(pipelineName_)), camLabel(std::move(camLabel_))
    , intrinsics(std::move(intrinsics_)), inputFormat(inputFormat_)
    , rawServer(streamFormat,std::format("{}_raw",pipelineName),rawPort)
    , processedServer(streamFormat,std::format("{}_processed",pipelineName),processedPort)
    , tagSize(tagSize_) {
        std::vector<std::unique_ptr<CVProcessNode<cv::Mat>>> nodes;
        if (streamFormat.frameFormat.cols != inputFormat.cols || streamFormat.frameFormat.rows != inputFormat.rows) {
            nodes.emplace_back(
                std::make_unique<ResizeNode<cv::Mat>>(
                    cv::INTER_LINEAR,
                    streamFormat.frameFormat.cols,
                    streamFormat.frameFormat.rows
                )
            );
        }
        nodes.emplace_back(
            std::make_unique<ColorConvertNode<cv::Mat>>(
                ColorSpace::GRAY,
                ColorSpace::COLOR
            )
        );
        prePostprocessor = std::make_unique<CVProcessPipe<cv::Mat>>(inputFormat,std::move(nodes));
        ntpub = NetworkTablesManager::getInstance().getDataPublisher(pipelineName);
    }

    // TODO: Add error codes & error handling
    int ApriltagPipelineStreamer::accept(Frame& frame, PipelineResult& result) noexcept {
        if (auto shared = ntpub.lock()) {
            shared->publishPipelineResult(result);
        }
        if (streamEnabled) {
            // TODO: Statically allocate a reusable buffer? Characterize this code. Right now (7/6/25) everyhting is still in prototyping, so this isn't super important atm.
            auto rframe = prePostprocessor->processFrame(frame);
            auto pframe = copyFrame(rframe);
            drawCamLabel(rframe.data,std::format("{} RAW",camLabel));
            rawServer.acceptFrame(rframe);
            drawCamLabel(pframe.data,std::format("{} PROCESSED",camLabel));
            for (const auto& tag : result.aprilTagDetections) {
                drawTag(pframe.data,tag);
            }
            for (const auto& tagpose: result.aprilTagPoses) {
                drawTag3D(pframe.data,tagpose,intrinsics,tagSize);
            }
        }
        return 0;
    }
}