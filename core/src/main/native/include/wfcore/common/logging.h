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

#include "wfcore/common/logging/LoggerManager.h"

#ifndef NDEBUG
    #define WF_DEBUGLOG(logger, fmt, ...) do { logger->debug(fmt __VA_OPT__(,) __VA_ARGS__); } while (0)
#else
    #define WF_DEBUGLOG(logger, fmt, ...) 
#endif

#define WF_LOGEXCEPT(exception) do {                            \
    wf::globalLogger()->error(                                  \
        "Exception caught at {}:{} in function {}: {}",         \
        __FILE__,                                               \
        __LINE__,                                               \
        __func__,                                               \
        exception.what());                                      \
} while (0)

namespace wf {
    inline loggerPtr& globalLogger() {
        static auto logger = LoggerManager::getInstance().getLogger("Wayfinder");
        return logger;
    }
}
