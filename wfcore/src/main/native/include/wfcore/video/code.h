// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>
#include <cstdint>

namespace wf::video {

    enum class Compression {
        RAW,
        H264,
        H265,
        MJPEG
    };

    // The methods below return void* to avoid having to expose the gst headers into wfcore's public API. To use them, cast the pointers to GstElement

    // Returns the default decoder for the compression, based on detected hardware and available codecs
    void* getDefaultDecoder(Compression compression);
    // Returns the default decoder for the compression, based on detected hardware and available codecs.
    void* getDefaultEncoder(Compression compression);

    // Returns a platform independent software decoder
    void* getUniversalDecoder(Compression compression);
    // Returns a platform independent software encoder
    void* getUniversalEncoder(Compression compression);
    
}