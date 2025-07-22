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

#include "wfcore/common/wfexcept.h"
#include <string>
#include <format>
#include <wfdef.h>
#include <iostream>

// This assertion macro should only be used OUTSIDE of the common directory.
// Inside the common directory, use regular C-style assertions, to avoid circular
// dependencies
#ifndef NDEBUG
#define WF_Assert(expr)                                                     \
    do {                                                                    \
        if (!expr) {                                                        \
            std::string msg                                                 \
                = std::format(                                              \
                    "Assertion failed: {} at {}:{} in function {}",         \
                    WF_TOSTRING(expr),                                      \
                    __FILE__,                                               \
                    __LINE__,                                               \
                    __func__                                                \
                );                                                          \
            auto& logger = globalLogger();                                  \
            if (logger) {                                                   \
                logger->error(msg);                                         \
            } else {                                                        \
                std::cerr << msg << std::endl;                              \
            }                                                               \
            throw wf::bad_assert(msg);                                      \
        }                                                                   \
    } while (0)
#else
#define WF_Assert(expr)
#endif
