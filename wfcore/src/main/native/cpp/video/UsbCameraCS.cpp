// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wfcore/video/UsbCameraCS.h"
#include <format>
#include <mutex>
#include <unordered_map>

namespace impl {
    static std::mutex cameraMutex;
    static std::unordered_map<std::string,cs::UsbCamera> cameras;

    static const cs::UsbCamera& getUsbCamera(const std::string& devPath) {
        std::lock_guard<std::mutex> lock(cameraMutex);
        auto it = cameras.find(devPath);
        if (it == cameras.end()) {
            cameras[devPath] = cs::UsbCamera(std::format("{}_source",devPath),devPath);
            return cameras[devPath];
        } else {
            return it->second;
        }
    }
}

namespace wf {

    UsbCameraCS::UsbCameraCS(const std::string& name_, const std::string& devPath_, FrameFormat format_)
    : format(format_), appSink(name_), devPath(devPath_), name(name_) {
        appSink.SetSource(impl::getUsbCamera(devPath));
    }

    Frame UsbCameraCS::getFrame() noexcept {
        cv::Mat imdata;
        uint64_t captimeMicros = appSink.GrabFrame(imdata);
        return Frame(captimeMicros,format,std::move(imdata));
    }

    const std::string& UsbCameraCS::getName() const noexcept {
        return name;
    }

    const std::string& UsbCameraCS::getDevPath() const noexcept {
        return devPath;
    }

    FrameFormat UsbCameraCS::getFormat() const noexcept {
        return format;
    }

}