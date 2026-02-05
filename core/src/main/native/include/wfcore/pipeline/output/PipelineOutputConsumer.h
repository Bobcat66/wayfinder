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

#include "wfcore/pipeline/Pipeline.h"
#include "wfcore/video/video_types.h"
#include "wfcore/common/status.h"

namespace wf {

    class ApriltagPipelineConsumer;
    class ObjdetectPipelineConsumer;
    struct PipelineOutputConsumerVisitor {
        virtual WFStatusResult operator()(ApriltagPipelineConsumer& outputConsumer) = 0;
        virtual WFStatusResult operator()(ObjdetectPipelineConsumer& outputConsumer) = 0;
    };


    class PipelineOutputConsumer {
    public:
        virtual ~PipelineOutputConsumer() = default;
        virtual bool consume(cv::Mat& data, FrameMetadata meta, PipelineResult& result) noexcept = 0;
        virtual PipelineType getPipelineType() const noexcept = 0;
        virtual WFStatusResult accept(PipelineOutputConsumerVisitor& visitor) = 0;
    };
}