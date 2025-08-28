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

#include "wfcore/pipeline/output/PipelineOutputConsumer.h"
#include "wfcore/video/video_types.h"
#include "wfcore/video/MJPEGVideoServerCS.h"
#include "wfcore/video/processing.h"
#include "wfcore/network/NTDataPublisher.h"
#include "wfcore/hardware/CameraConfiguration.h"
#include <memory>

namespace wf {

    // An apriltag pipeline output consumer that posts to networktables and streams video to HTTP
    class ApriltagPipelineConsumer : public PipelineOutputConsumer {
    public:
        ApriltagPipelineConsumer(
            std::string pipelineName_, std::string camLabel_, 
            CameraIntrinsics intrinsics_, FrameFormat inputFormat_,
            int rawPort, int processedPort, 
            StreamFormat streamFormat,
            double tagSize_, std::weak_ptr<NTDataPublisher> ntpub_
        );
        bool consume(cv::Mat& data, FrameMetadata meta, PipelineResult& result) noexcept override;
        PipelineType getPipelineType() const noexcept override { return PipelineType::Apriltag; }
        WFStatusResult accept(PipelineOutputConsumerVisitor& visitor) override { return visitor(*this); }
        int getRawPort();
        int getProcessedPort();
        StreamFormat getOutputFormat() { return outputFormat; }
        void enableStreaming(bool streamingEnabled) noexcept { this->streamingEnabled = streamingEnabled; }
        bool isStreaming() const noexcept { return this->streamingEnabled; }
    private:
        FrameFormat inputFormat;
        CameraIntrinsics intrinsics;
        double tagSize;
        MJPEGVideoServerCS rawServer;
        MJPEGVideoServerCS processedServer;
        StreamFormat outputFormat;
        std::weak_ptr<NTDataPublisher> ntpub;
        std::string pipelineName;
        std::string camLabel;
        std::unique_ptr<CVProcessPipe<cv::Mat>> prePostprocessor;
        cv::Mat pp_rawbuf;
        cv::Mat pp_procbuf;
        bool streamingEnabled;
    };
}