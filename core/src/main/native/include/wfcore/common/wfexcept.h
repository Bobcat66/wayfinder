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

#include <stdexcept>
#include <string>

// Macro that automatically defines a subclass of std::runtime_error
#define DEF_RUNEXCEPT(exceptionName,defaultMsg)                 \
    class exceptionName : public std::runtime_error {       \
    public:                                                 \
        exceptionName()                                     \
        : std::runtime_error(defaultMsg) {}                 \
        explicit exceptionName(const std::string& msg)      \
        : std::runtime_error(msg) {}                        \
    };

namespace wf {
    DEF_RUNEXCEPT(invalid_pipeline_configuration,"Invalid pipeline configuration")
    DEF_RUNEXCEPT(camera_not_found,"Camera Not Found")
    DEF_RUNEXCEPT(intrinsics_not_found,"Intrinsics not found for camera at specified resolution")
    DEF_RUNEXCEPT(vision_worker_not_found,"Vision Worker not found")
}

#undef DEF_RTERR