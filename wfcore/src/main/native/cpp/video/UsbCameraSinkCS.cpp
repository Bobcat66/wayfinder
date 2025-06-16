// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wfcore/video/UsbCameraSinkCS.h"
#include <format>
#include <shared_mutex>
#include <mutex>
#include <unordered_map>

namespace impl {
    static std::shared_mutex cameraMutex;
    static std::unordered_map<std::string,std::shared_ptr<cs::UsbCamera>> cameras;
    static std::unordered_map<std::string,wf::StreamFormat> streamFormats;

    static wf::StreamFormat getUsbCameraStreamFormat(cs::UsbCamera& camera) {
        std::shared_lock lock(cameraMutex);
        auto vmode = camera.GetVideoMode();
        lock.unlock();
        wf::ColorSpace cspace;
        switch (vmode.pixelFormat) {
            case cs::VideoMode::PixelFormat::kBGR:
                cspace = wf::ColorSpace::COLOR;
                break;
            case cs::VideoMode::PixelFormat::kGray:
                cspace = wf::ColorSpace::GRAY;
                break;
            case cs::VideoMode::PixelFormat::kY16:
                cspace = wf::ColorSpace::DEPTH;
                break;
            default:
                cspace = wf::ColorSpace::UNKNOWN;
        }
        return {
            vmode.fps,
            {
                cspace,
                vmode.height,
                vmode.width
            }
        };
    }


    static const std::shared_ptr<cs::UsbCamera> getUsbCamera(const std::string& devPath) {
        std::unique_lock lock(cameraMutex);
        auto it = cameras.find(devPath);
        if (it == cameras.end()) {
            cameras[devPath] = std::make_shared<cs::UsbCamera>(std::format("{}_source",devPath),devPath);
            streamFormats[devPath] = getUsbCameraStreamFormat(*cameras[devPath]);
            return cameras[devPath];
        } else {
            return it->second;
        }
    }

    static bool applyStreamFormatToUsbCamera(std::shared_ptr<cs::UsbCamera> camera, wf::StreamFormat sformat) {
        cs::VideoMode::PixelFormat pfmt;
        switch (sformat.frameFormat.colorspace) {
            case (wf::ColorSpace::COLOR):
                pfmt = cs::VideoMode::PixelFormat::kBGR;
                break;
            case (wf::ColorSpace::GRAY):
                pfmt = cs::VideoMode::PixelFormat::kGray;
                break;
            case (wf::ColorSpace::DEPTH):
                pfmt = cs::VideoMode::PixelFormat::kY16;
                break;
        }
        cs::VideoMode newVideoMode{
            pfmt,
            sformat.frameFormat.cols,
            sformat.frameFormat.rows,
            sformat.fps
        };
        std::unique_lock lock(cameraMutex);
        return camera->SetVideoMode(newVideoMode);
    }
}

namespace wf {

    UsbCameraSinkCS::UsbCameraSinkCS(const std::string& name_, const std::string& devPath_)
    : appSink(name_), devPath(devPath_), name(name_) {
        camera = impl::getUsbCamera(devPath);
        appSink.SetSource(*camera);
    }

    Frame UsbCameraSinkCS::getFrame() noexcept {
        cv::Mat imdata;
        uint64_t captimeMicros = appSink.GrabFrame(imdata);
        return Frame(
            captimeMicros,
            impl::getUsbCameraStreamFormat(*camera).frameFormat,
            std::move(imdata)
        );
    }

    const std::string& UsbCameraSinkCS::getName() const noexcept {
        return name;
    }

    const std::string& UsbCameraSinkCS::getDevPath() const noexcept {
        return devPath;
    }

    StreamFormat UsbCameraSinkCS::getStreamFormat() const noexcept {
        return impl::getUsbCameraStreamFormat(*camera);
    }

    bool setUsbCameraFormat(const std::string& devPath, StreamFormat sformat) {
        return impl::applyStreamFormatToUsbCamera(
            impl::getUsbCamera(devPath),
            sformat
        );
    }

}