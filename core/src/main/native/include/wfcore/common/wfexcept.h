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

// Macro that automatically defines a subclass of std::exception. Warning: not thread-safe by default
#define WF_DEFEXCEPT(exceptionName,statusCode,defaultMsg)                                       \
    class exceptionName : public wfexception {                                                  \
        using enum WFStatus;                                                                    \
    public:                                                                                     \
        exceptionName()                                                                         \
        : msg(defaultMsg) {}                                                                    \
        template <typename... Args>                                                             \
        exceptionName(std::string_view fmt, Args&&... args) {                                   \
            try {                                                                               \
                msg = std::vformat(fmt, std::make_format_args(args...));                        \
            } catch (const std::format_error& e) {                                              \
                msg = "MSGERR bad_format";                                                      \
            }                                                                                   \
        }                                                                                       \
        const char* what() const noexcept override {                                            \
            if (fullmsg.empty()) fullmsg = std::format(                                         \
                "{} ({:#10x}): {}",                                                             \
                wfstatus_name(status()),                                                        \
                static_cast<uint32_t>(status()),                                                \
                msg                                                                             \
            );                                                                                  \
            return fullmsg.c_str();                                                             \
        }                                                                                       \
        WFStatus status() const noexcept override {                                             \
            return statusCode;                                                                  \
        }                                                                                       \
    private:                                                                                    \
        std::string msg;                                                                        \
        mutable std::string fullmsg;                                                            \
    };

namespace wf {
    class wfexception : public std::exception {
    public:
        virtual const char* what() const noexcept override = 0;
        virtual ~wfexception() = default;
        virtual WFStatus status() const noexcept = 0;
    };

    // TODO: refactor these to use actual status codes
    WF_DEFEXCEPT(bad_assert,BAD_ASSERT,"Bad assertion")
    WF_DEFEXCEPT(bad_local_dir,CONFIG_BAD_LOCALDIR,"Bad local directory")
    WF_DEFEXCEPT(bad_resource_dir,CONFIG_BAD_RESOURCEDIR,"Bad resource directory")
    WF_DEFEXCEPT(bad_model,INFERENCE_BAD_MODEL,"Bad model")
    WF_DEFEXCEPT(invalid_pipeline_configuration,UNKNOWN,"Invalid pipeline configuration")
    WF_DEFEXCEPT(camera_not_found,UNKNOWN,"Camera not found")
    WF_DEFEXCEPT(intrinsics_not_found,UNKNOWN,"Intrinsics not found")
    WF_DEFEXCEPT(vision_worker_not_found,UNKNOWN,"Vision worker not found")
    WF_DEFEXCEPT(invalid_camera_control,UNKNOWN,"Invalid camera control")
    WF_DEFEXCEPT(invalid_stream_format,UNKNOWN,"Invalid stream format")
    WF_DEFEXCEPT(invalid_image_encoding,UNKNOWN,"Invalid image encoding")
    WF_DEFEXCEPT(invalid_engine_type,UNKNOWN,"Invalid engine type")
    WF_DEFEXCEPT(invalid_model_arch,UNKNOWN,"Invalid model architecture")
    WF_DEFEXCEPT(model_not_loaded,UNKNOWN,"Model not loaded")
    WF_DEFEXCEPT(failed_resource_acquisition,UNKNOWN,"Failed resource acquisition")
    WF_DEFEXCEPT(json_error,UNKNOWN,"JSON Error")
    WF_DEFEXCEPT(unknown_exception,UNKNOWN,"Unknown exception")

    class wf_result_error : public wfexception {
    public:
        template <typename T>
        wf_result_error(WFResult<T> result) 
        : status_(result.status()), msg_(result.what()) {}
        const char* what() const noexcept override { return msg_.c_str(); }
        WFStatus status() const noexcept override { return status_; }
    private:
        std::string msg_;
        WFStatus status_;
    };
}

#undef WF_DEFEXCEPT