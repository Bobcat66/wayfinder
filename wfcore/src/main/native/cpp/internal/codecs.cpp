// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#include "impl/codecs.h"
#include "impl/gsthelpers.h"
#include <gst/gst.h>
#include "wfcore/logging/LoggerManager.h"
#include <string>
#include <unordered_map>

namespace wf::impl {
    
    using wf::video::Compression;

    static loggerPtr logger = wf::LoggerManager::getInstance().getLogger("gst-internal-codecs",wf::LogGroup::Gstreamer);

    // TODO: make hardware-accelerated codecs the default
    GstElement* getDefaultDecoder(Compression compression, const std::string& instanceName){
        return getSoftwareDecoder(compression, instanceName);
    }

    // this returns a fallback software decoder, if possible
    GstElement* getSoftwareDecoder(Compression compression, const std::string& instanceName) {
        switch (compression) {
            case Compression::RAW:
                return nullptr;
            case Compression::MJPEG:
                if (!findGSTPlugin("jpeg")) {
                    logger->warn("jpeg plugin not installed, MJPEG software codecs will NOT be available");
                }
                return getGSTElement("jpegdec",instanceName);
            case Compression::H264:
                if (!findGSTPlugin("openh264")) {
                    logger->warn("openh264 plugin not installed, H264 software codecs will NOT be available");
                }
                return wf::impl::getGSTElement("openh264dec",instanceName);
            case Compression::H265:
                if (!wf::impl::findGSTPlugin("de265")) {
                    logger->warn("de265 plugin not installed, H265 software decoding will NOT be available");
                }
                return nullptr;
        }
    }
    // TODO: make hardware-accelerated codecs the default
    GstElement* getDefaultEncoder(Compression compression, const std::string& instanceName) {
        return getSoftwareEncoder(compression, instanceName); // Placeholder
    }

    GstElement* getSoftwareEncoder(Compression compression, const std::string& instanceName) {
        switch (compression) {
            case Compression::RAW:
                return nullptr;
            case Compression::MJPEG:
                if (wf::impl::findGSTPlugin("jpeg")) {
                    logger->warn("jpeg plugin not installed, MJPEG software codecs will NOT be available");
                }
                return wf::impl::getGSTElement("jpegenc",instanceName);
            case Compression::H264:
                if (!wf::impl::findGSTPlugin("openh264")) {
                    logger->warn("openh264 plugin not installed, H264 software codecs will NOT be available");
                }
                return wf::impl::getGSTElement("openh264enc",instanceName);
            case Compression::H265:
                logger->warn("H265 software encoders are not available");
                return nullptr;
        }
    }

}



