// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "wfcore/video/video_types.h"

#include <string>

namespace wf {
    class FrameProvider {
    public:
        virtual Frame getFrame() noexcept = 0;
        virtual ~FrameProvider() noexcept = default;
        virtual const std::string& getName() const noexcept = 0;
        virtual StreamFormat getStreamFormat() const noexcept = 0;
    };
}