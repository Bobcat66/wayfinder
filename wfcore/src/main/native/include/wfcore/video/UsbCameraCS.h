// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "wfcore/video/FrameProvider.h"
#include <cscore_oo.h>
#include <cscore_cv.h>
#include <memory>

// A CSCore-based implementation of FrameProvider, which pulls frames from a USB camera 
namespace wf {
    class UsbCameraCS : FrameProvider {
        public:
        UsbCameraCS(
            const std::string& name, 
            const std::string& devPath,
            FrameFormat format_
        );
        ~UsbCameraCS() override = default;
        Frame getFrame() noexcept override;
        const std::string& getName() const noexcept override;
        const std::string& getDevPath() const noexcept;
        FrameFormat getFormat() const noexcept;
        private:
        const FrameFormat format; // The format of the frames returned by this FrameProvider
        cs::CvSink appSink;
        std::string devPath;
        std::string name;
    };
}