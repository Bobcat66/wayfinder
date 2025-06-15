// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "wfcore/video/FrameProvider.h"
#include <cscore_oo.h>
#include <cscore_cv.h>
#include <memory>

// A CSCore-based implementation of FrameProvider, which pulls frames from a USB camera 
namespace wf {
    class UsbCameraSinkCS : public FrameProvider {
        public:
        UsbCameraSinkCS(const std::string& name, const std::string& devPath);
        ~UsbCameraSinkCS() override = default;
        Frame getFrame() noexcept override;
        const std::string& getName() const noexcept override;
        const std::string& getDevPath() const noexcept;
        StreamFormat getStreamFormat() const noexcept;
        private:
        cs::CvSink appSink;
        std::string devPath;
        std::string name;
        std::shared_ptr<cs::UsbCamera> camera;
    };

    bool setUsbCameraFormat(const std::string& devPath, FrameFormat format);
}