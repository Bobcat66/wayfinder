// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#include "wfcore/video/code.h"
#include <gst/gst.h>
#include <string>
#include <unordered_map>

using namespace wf::video;

// TODO: set up hardware identification and make hardware-accelerated codecs the default
void* getDefaultEncoder(Compression compression) {
    switch (compression) {
        case Compression::RAW:
            return "None";
        case Compression::MJPEG:
            return ""
    }
}

// If the specified codec is unavailable, this returns a fallback software encoder
void* getUniversalEncoder(Compression compression) {
    switch (compression) {
        case Compression::RAW:
            return nullptr;
        case Compression::MJPEG:
            return gst
    }
}



