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

#include "wfcore/video/video_types.h"
#include "wfcore/common/status/ConcurrentStatusfulObject.h"

#include <string>
#include <opencv2/core.hpp>

namespace wf {
    // TODO: Make more statuses and refactor frame provider code to use them
    enum class FrameProviderStatus {
        Ok
    };

    class FrameProvider : public ConcurrentStatusfulObject<FrameProviderStatus,FrameProviderStatus::Ok>{
    public:
        virtual FrameMetadata getFrame(cv::Mat& mat) = 0;
        virtual ~FrameProvider() noexcept = default;
        virtual const std::string& getName() const noexcept = 0;
        virtual const StreamFormat& getStreamFormat() const noexcept = 0;
    };
}