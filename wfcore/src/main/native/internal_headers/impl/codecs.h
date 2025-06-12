// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "wfcore/video/formats.h"

#include <gst/gst.h>

#include <string>
#include <cstdint>

namespace wf::impl {

    // Returns the default decoder for the compression, based on detected hardware and available codecs
    GstElement* getDefaultDecoder(wf::video::Compression compression, const std::string& instanceName);
    // Returns the default decoder for the compression, based on detected hardware and available codecs.
    GstElement* getDefaultEncoder(wf::video::Compression compression, const std::string& instanceName);

    // Returns a platform independent software decoder
    GstElement* getSoftwareDecoder(wf::video::Compression compression, const std::string& instanceName);
    // Returns a platform independent software encoder
    GstElement* getSoftwareEncoder(wf::video::Compression compression, const std::string& instanceName);
    
}