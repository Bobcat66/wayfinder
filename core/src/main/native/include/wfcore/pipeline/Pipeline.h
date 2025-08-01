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

#include "wfcore/types.h"
#include "wfcore/video/video_types.h"
#include "wfcore/pipeline/PipelineType.h"
#include "wfcore/pipeline/PipelineResult.h"
#include "wfcore/common/status.h"
#include <vector>
#include <optional>
#include <cstdint>

namespace wf {

    class Pipeline {
    public:
        [[nodiscard]] 
        virtual WFResult<PipelineResult> process(const cv::Mat& data, const FrameMetadata& meta) noexcept = 0;
        virtual ~Pipeline() = default;
    };
    
}
