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

namespace impl {
    using namespace wf;
    static const std::unordered_map<std::string,ImageEncoding> encodingMap = {
        {"BGR24",ImageEncoding::BGR24},
        {"RGB24",ImageEncoding::RGB24},
        {"RGB565",ImageEncoding::RGB565},
        {"Y8",ImageEncoding::Y8},
        {"Y16",ImageEncoding::Y16},
        {"YUYV",ImageEncoding::YUYV},
        {"UYVY",ImageEncoding::UYVY},
        {"RGBA",ImageEncoding::RGBA},
        {"BGRA",ImageEncoding::BGRA},
        {"MJPEG",ImageEncoding::MJPEG},
        {"UNKNOWN",ImageEncoding::UNKNOWN}
    };

    static const std::unordered_map<ImageEncoding,std::string> encodingStringMap = {
        {ImageEncoding::BGR24,"BGR24"},
        {ImageEncoding::RGB24,"RGB24"},
        {ImageEncoding::RGB565,"RGB565"},
        {ImageEncoding::Y8,"Y8"},
        {ImageEncoding::Y16,"Y16"},
        {ImageEncoding::YUYV,"YUYV"},
        {ImageEncoding::UYVY,"UYVY"},
        {ImageEncoding::RGBA,"RGBA"},
        {ImageEncoding::BGRA,"BGRA"},
        {ImageEncoding::MJPEG,"MJPEG"},
        {ImageEncoding::UNKNOWN,"UNKNOWN"}
    };
}

namespace wf {

    using enum JSONStatus;

    JSONStatusResult<JSONObject> FrameFormat::toJSON_impl(const FrameFormat& object) {
        try {
            JSONObject jobject = {
                {"width",object.cols},
                {"height",object.rows},
                {"encoding",impl::encodingStringMap.at(object.encoding)}
            };
            return JSONStatusResult<JSONObject>::success(std::move(jobject));
        } catch (const nlohmann::json::exception& e) {
            jsonLogger()->error("Error while serializing FrameFormat: {}",e.what());
            return JSONStatusResult<JSONObject>::failure(Unknown);
        }
    }

    JSONStatusResult<FrameFormat> FrameFormat::fromJSON_impl(const JSONObject& jobject) {
        if (!validateProperties(jobject,{"width","height","encoding"},"frame_format"))
            return JSONStatusResult<FrameFormat>::failure(PropertyNotFound);
        int width,height;
        std::string encodingString;
        try {
            width = jobject["width"].get<int>();
            height = jobject["height"].get<int>();
            encodingString = jobject["encoding"].get<std::string>();
        } catch (const nlohmann::json::type_error& e) {
            jsonLogger()->error("Type error while parsing frame_format: {}",e.what());
            return JSONStatusResult<FrameFormat>::failure(InvalidType);
        } catch (const nlohmann::json::exception& e) {
            jsonLogger()->error("JSON error while parsing frame_format: {}",e.what());
            return JSONStatusResult<FrameFormat>::failure(Unknown);
        }

        auto it = impl::encodingMap.find(encodingString);
        if (it == impl::encodingMap.end()) {
            jsonLogger()->error("Schema violation while parsing frame_format: '{}' is not a recognized image encoding",encodingString);
            return JSONStatusResult<FrameFormat>::failure(SchemaViolation);
        }

        return JSONStatusResult<FrameFormat>::success(std::in_place,it->second,width,height);
        
    }

    JSONStatusResult<JSONObject> StreamFormat::toJSON_impl(const StreamFormat& object) {
        JSONObject frameFormat_jobject;
        if (auto jresult = FrameFormat::toJSON(object.frameFormat)) {
            frameFormat_jobject = std::move(jresult.value());
        } else {
            jsonLogger()->error("Error while parsing StreamFormat.frameFormat");
            return JSONStatusResult<JSONObject>::failure(jresult.status());
        }

        try {
            JSONObject jobject = {
                { "fps",object.fps },
                { "frameFormat",std::move(frameFormat_jobject) }
            };
            return JSONStatusResult<JSONObject>::success(std::move(jobject));
        } catch (const nlohmann::json::exception& e) {
            jsonLogger()->error("Error while serializing StreamFormat: {}",e.what());
            return JSONStatusResult<JSONObject>::failure(Unknown);
        }
    }

    JSONStatusResult<StreamFormat> StreamFormat::fromJSON_impl(const JSONObject& jobject) {
        if (!validateProperties(jobject,{"fps","frameFormat"},"stream_format")) {
            return JSONStatusResult<StreamFormat>::failure(PropertyNotFound);
        }

        if (!jobject["frameFormat"].is_object()) {
            jsonLogger()->error("stream_format.frameFormat is not an object");
            return JSONStatusResult<StreamFormat>::failure(InvalidType);
        }

        FrameFormat frameFormat;
        if (auto jresult = FrameFormat::fromJSON(jobject["frameFormat"])) {
            frameFormat = std::move(jresult.value());
        } else {
            jsonLogger()->error("Error while parsing stream_format.frameFormat");
            return JSONStatusResult<StreamFormat>::failure(jresult.status());
        }

        int fps;
        try {
            fps = jobject["fps"].get<int>();
        } catch (const nlohmann::json::type_error& e) {
            jsonLogger()->error("Type error while parsing stream_format: {}",e.what());
            return JSONStatusResult<StreamFormat>::failure(InvalidType);
        } catch (const nlohmann::json::exception& e) {
            jsonLogger()->error("JSON error while parsing stream_format: {}",e.what());
            return JSONStatusResult<StreamFormat>::failure(Unknown);
        }

        return JSONStatusResult<StreamFormat>::success(std::in_place,fps,std::move(frameFormat));
    }
}