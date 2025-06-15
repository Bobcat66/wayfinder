// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "wfcore/video/video_types.h"

#include <string>

namespace wf {
    class FrameProvider {
        public:
        virtual Frame getFrame() noexcept;
        virtual ~FrameProvider() noexcept = default;
        virtual const std::string& getName() const noexcept;
        virtual StreamFormat getFrameFormat() const noexcept;
    };
}