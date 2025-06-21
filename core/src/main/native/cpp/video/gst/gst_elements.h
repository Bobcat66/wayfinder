/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of Wayfinder.
 *
 * Copyright (C) 2025 Jesse Kane
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#ifdef WF_GSTREAMER

#include <string>
#include <unordered_map>

namespace impl {

    enum class Compression {
        RAW,
        H264,
        H265,
        MJPEG
    };

    enum class ColorSpace {
        GRAY8,
        Z16,
        Y16,
        YUY2,
        UYUV,
        NV12
    };

    struct Codec {
        Compression compression;
        std::string encGSTName;
        std::string decGSTName;
    };

    const std::unordered_map<std::string,Codec> codecs = {
        {"RAW",{Compression::RAW,"NONE","NONE"}},

        {"gstMJPEG",{Compression::MJPEG,"jpegenc","jpegdec"}}, // Gstreamer software MJPEG codec
        {"nvidiaMJPEG",{Compression::MJPEG,"nvjpegenc","nvjpegdec"}}, // Nvidia hardware MJPEG codec
        {"vaapiMJPEG",{Compression::MJPEG,"vajpegenc","vajpegdec"}}, // VA-API hardware MJPEG codec
        {"ffmpegMJPEG",{Compression::MJPEG,"avenc_mjpeg","avdec_mjpeg"}}, // FFmpeg software MJPEG codec
        {"v4l2MJPEG",{Compression::MJPEG,"v4l2jpegenc","v4l2jpegdec"}}, // Video4Linux2 M2M hardware MJPEG codec (test this)
        {"intelMJPEG",{Compression::MJPEG,"qsvjpegenc","qsvjpegdec"}}, // Intel Quick Sync hardware MJPEG codec

        {"openH264",{Compression::H264,"openh264enc","openh264dec"}}, // Cisco's open-source software h.264 codec
        {"nvidiaH264",{Compression::H264,"nvh264enc","nvh265dec"}}, // Nvidia hardware h.264 codec
        {"ffmpegH264dec",{Compression::H264,"NONE","avdec_h264"}}, // FFmpeg software h.264 decode
        {"dx11H264dec",{Compression::H264,"NONE","d3d11h264dec"}}, // DirectX 11 hardware h.264 decode
        {"dx12H264",{Compression::H264,"d3d12h264enc","d3d12h264dec"}}, // DirectX 12 hardware h.264 codec
        {"v4l2H264",{Compression::H264,"v4l2h264enc","v4l2h264dec"}}, // Video4Linux2 M2M hardware h.264 codec (test this)
        {"vaapiH264",{Compression::H264,"vah264enc","vah264dec"}}, // VA-API hardware h.264 codec

        {"ffmpegH265dec",{Compression::H265,"NONE","avdec_h265"}}, // FFmpeg software h.265 decode
        {"nvidiaH265",{Compression::H265,"nvh265enc","nvh265dec"}}, // Nvidia hardware h.265 codec
    };

    // These elements convert between color-spaces
    const std::unordered_map<std::string,std::string> colorConverters = {
        {"gstColor","videoconvert"}, // Gstreamer software color-space conversion
        {"openglColor","glcolorconvert"}, // OpenGL color-space conversion
        {"v4l2Color","v4l2convert"}, // V4L2 hardware color-space conversion (Test this)
        {"nvidiaColor","cudaconvert"}, // Nvidia CUDA color-space conversion
        {"dx11Color","d3d11colorconvert"}, // DirectX 11 color-space conversion
        {"dx12Color","d3d11colorconvert"}, // DirectX 12 color-space conversion
        {"vulkanColor","vulkancolorconvert"}, // Vulkan color space conversion
    };  
}
#endif // WF_GSTREAMER