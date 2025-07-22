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
#include "wfcore/common/status.h"

#ifdef WF_DEFEXCEPT
    #error "WF_DEFEXCEPT is defined outside of wfexcept.h, causing a macro name collision. Aborting build"
#endif

// Macro that automatically defines a subclass of std::exception
#define WF_DEFEXCEPT(exceptionName,statusCode)                                                  \
    class exceptionName : public wfexception<exceptionName> {                                   \
        using enum WFStatus;                                                                    \
    public:                                                                                     \
        exceptionName()                                                                         \
        : msg(wfstatus_name(statusCode)) {}                                                     \
        template <typename... Args>                                                             \
        exceptionName(std::string_view fmt, Args&&... args)                                     \
        : msg(std::vformat(fmt,std::make_format_args(std::forward<Args>(args)...))) {}          \
        const char* what() const noexcept override {                                            \
            return msg.c_str();                                                                 \
        }                                                                                       \
        static constexpr WFStatus status_impl() noexcept {                                      \
            return statusCode;                                                                  \
        }                                                                                       \
    private:                                                                                    \
        std::string msg;                                                                        \
    };

namespace wf {
    template <typename T>
    class wfexception : public std::exception {
    public:
        virtual const char* what() const noexcept override = 0;
        virtual ~wfexception() = default;
        static constexpr WFStatus status() noexcept {
            return T::status_impl();
        }
    };

    // TODO: refactor these to use actual status codes
    WF_DEFEXCEPT(invalid_pipeline_configuration,UNKNOWN)
    WF_DEFEXCEPT(camera_not_found,UNKNOWN)
    WF_DEFEXCEPT(intrinsics_not_found,UNKNOWN)
    WF_DEFEXCEPT(vision_worker_not_found,UNKNOWN)
    WF_DEFEXCEPT(invalid_camera_control,UNKNOWN)
    WF_DEFEXCEPT(invalid_stream_format,UNKNOWN)
    WF_DEFEXCEPT(invalid_image_encoding,UNKNOWN)
    WF_DEFEXCEPT(invalid_engine_type,UNKNOWN)
    WF_DEFEXCEPT(invalid_model_arch,UNKNOWN)
    WF_DEFEXCEPT(model_not_loaded,UNKNOWN)
    WF_DEFEXCEPT(failed_resource_acquisition,UNKNOWN)
    WF_DEFEXCEPT(json_error,UNKNOWN)
    WF_DEFEXCEPT(unknown_exception,UNKNOWN)
}

#undef WF_DEFEXCEPT