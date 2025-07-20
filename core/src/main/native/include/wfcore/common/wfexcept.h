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


#pragma once

#include <exception>
#include <string>
#include <format>

#ifdef WF_DEFEXCEPT
    #error "WF_DEFEXCEPT is defined outside of wfexcept.h, causing a macro name collision. Aborting build"
#endif

// Macro that automatically defines a subclass of std::exception
#define WF_DEFEXCEPT(exceptionName,defaultMsg)                                                  \
    class exceptionName : public wfexception {                                                  \
    public:                                                                                     \
        exceptionName() : msg(defaultMsg) {}                                                    \
        template <typename... Args>                                                             \
        exceptionName(std::string_view fmt, Args&&... args)                                     \
        : msg(std::vformat(fmt,std::make_format_args(std::forward<Args>(args)...))) {}          \
        const char* what() const noexcept override {                                            \
            return msg.c_str();                                                                 \
        }                                                                                       \
    private:                                                                                    \
        std::string msg;                                                                        \
    };

namespace wf {
    class wfexception : public std::exception {
    public:
        virtual const char* what() const noexcept override = 0;
        virtual ~wfexception() = default;
    };
    WF_DEFEXCEPT(invalid_pipeline_configuration,"Invalid pipeline configuration")
    WF_DEFEXCEPT(camera_not_found,"Camera Not Found")
    WF_DEFEXCEPT(intrinsics_not_found,"Intrinsics not found for camera at specified resolution")
    WF_DEFEXCEPT(vision_worker_not_found,"Vision Worker not found")
    WF_DEFEXCEPT(invalid_camera_control,"Invalid camera control")
    WF_DEFEXCEPT(invalid_stream_format,"Invalid stream format")
    WF_DEFEXCEPT(invalid_image_encoding,"Invalid image encoding")
    WF_DEFEXCEPT(invalid_engine_type,"Invalid engine type")
    WF_DEFEXCEPT(invalid_model_arch,"Invalid model architecture")
    WF_DEFEXCEPT(model_not_loaded,"Failed to load model")
    WF_DEFEXCEPT(failed_resource_acquisition,"Failed to acquire resources")
    WF_DEFEXCEPT(json_error,"JSON Error")
    WF_DEFEXCEPT(unknown_exception,"Unknown exception")
}

#undef WF_DEFEXCEPT