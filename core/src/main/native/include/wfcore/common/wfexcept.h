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

// Macro that automatically defines a subclass of std::exception
#define DEFEXCEPT(exceptionName,defaultMsg)                                                     \
    class exceptionName : public std::exception {                                               \
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
    DEFEXCEPT(invalid_pipeline_configuration,"Invalid pipeline configuration")
    DEFEXCEPT(camera_not_found,"Camera Not Found")
    DEFEXCEPT(intrinsics_not_found,"Intrinsics not found for camera at specified resolution")
    DEFEXCEPT(vision_worker_not_found,"Vision Worker not found")
    DEFEXCEPT(invalid_camera_control,"Invalid camera control")
    DEFEXCEPT(invalid_stream_format,"Invalid stream format")
    DEFEXCEPT(unknown_exception,"Unknown")
}

#undef DEFEXCEPT