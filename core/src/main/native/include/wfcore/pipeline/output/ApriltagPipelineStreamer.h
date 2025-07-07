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
#include "wfcore/configuration/CameraConfiguration.h"
#include <memory>

namespace wf {

    // An apriltag pipeline output consumer that posts to networktables and streams video to HTTP
    class ApriltagPipelineStreamer : public PipelineOutputConsumer {
    public:
        ApriltagPipelineStreamer(
            std::string pipelineName_, std::string camLabel_, 
            CameraIntrinsics intrinsics_, FrameFormat inputFormat_,
            int rawPort, int processedPort, 
            StreamFormat streamFormat,
            double tagSize_
        );
        int accept(Frame& frame, PipelineResult& result) noexcept override;
        PipelineType getPipelineType() override { return PipelineType::Apriltag; }
        void enableStream(bool streamEnabled) { this->streamEnabled = streamEnabled; }
        bool isStreaming() { return this->streamEnabled; }
    private:
        FrameFormat inputFormat;
        CameraIntrinsics intrinsics;
        double tagSize;
        MJPEGVideoServerCS rawServer;
        MJPEGVideoServerCS processedServer;
        std::weak_ptr<NTDataPublisher> ntpub;
        bool streamEnabled;
        std::string pipelineName;
        std::string camLabel;
        std::unique_ptr<CVProcessPipe<cv::Mat>> prePostprocessor;
    };
}