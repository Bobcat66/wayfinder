/*
 * SPDX-License-Identifier: GPL-3.0-or-later
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

#include "wfcore/video/video_types.h"
#include "wfcore/common/wfexcept.h"
#include <unordered_map>
#include "jval/StreamFormat.jval.hpp"

namespace impl {
    using namespace wf;

    inline ImageEncoding parseEncoding(const std::string& name) {
        if (name == "BGR24") return ImageEncoding::BGR24;
        if (name == "RGB24") return ImageEncoding::RGB24;
        if (name == "RGB565") return ImageEncoding::RGB565;
        if (name == "Y8") return ImageEncoding::Y8;
        if (name == "Y16") return ImageEncoding::Y16;
        if (name == "YUYV") return ImageEncoding::YUYV;
        if (name == "UYVY") return ImageEncoding::UYVY;
        if (name == "RGBA") return ImageEncoding::RGBA;
        if (name == "BGRA") return ImageEncoding::BGRA;
        if (name == "MJPEG") return ImageEncoding::MJPEG;
        return ImageEncoding::UNKNOWN;
    }

    constexpr std::string_view encodingToString(ImageEncoding encoding) {
    using enum ImageEncoding;
    switch (encoding) {
        case BGR24: return "BGR24";
        case RGB24: return "RGB24";
        case RGB565: return "RGB565";
        case Y8: return "Y8";
        case Y16: return "Y16";
        case YUYV: return "YUYV";
        case UYVY: return "UYVY";
        case RGBA: return "RGBA";
        case BGRA: return "BGRA";
        case MJPEG: return "MJPEG";
        default: return "UNKNOWN";
    }
}
}

namespace wf {

    using enum WFStatus;

    const jval::JSONValidationFunctor* StreamFormat::getValidator_impl() {
        return jval::get_StreamFormat_validator();
    }

    WFResult<JSON> StreamFormat::toJSON_impl(const StreamFormat& object) {

        try {
            JSON jobject = {
                { "fps",object.fps },
                { "frameFormat", {
                    {"width",object.frameFormat.width},
                    {"height",object.frameFormat.height},
                    {"encoding",impl::encodingToString(object.frameFormat.encoding)}
                }}
            };
            return WFResult<JSON>::success(std::move(jobject));
        } catch (const nlohmann::json::exception& e) {
            jsonLogger()->error("Error while serializing StreamFormat: {}",e.what());
            return WFResult<JSON>::failure(JSON_UNKNOWN);
        }
    }

    WFResult<StreamFormat> StreamFormat::fromJSON_impl(const JSON& jobject) {

        auto valid = validate(jobject);
        if (!valid) return WFResult<StreamFormat>::propagateFail(valid);

        auto ff_jobject = jobject["frameFormat"];
        return WFResult<StreamFormat>::success(
            std::in_place,
            jobject["fps"].get<int>(),
            FrameFormat{
                impl::parseEncoding(ff_jobject["encoding"].get<std::string>()),
                ff_jobject["width"].get<int>(),
                ff_jobject["height"].get<int>()
            }
        );
    }
}