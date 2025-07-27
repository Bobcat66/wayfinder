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

// This file CANNOT include any wayfinder headers, it is part of the basic startup system

// These methods will only be called outside the hot path. Robustness, expressiveness, and code
// readability should be emphasized over performance

#pragma once

#include <cstdlib>
#include <optional>
#include <iostream>
#include <string_view>
#include <cstdint>
#include <vector>

namespace wf::env {

    // Returns a copy of an environment variable string
    std::optional<std::string> getVar(const char* key, bool verbose = false);

    std::optional<int> getInt(const char* key, bool verbose = false);

    std::optional<double> getDouble(const char* key, bool verbose = false);

    std::optional<bool> getBool(const char* key, bool verbose = false);

    // Retrieves an integer array serialized as a JSON string from the environment variables
    std::optional<std::vector<int>> getIntArr(const char* key, bool verbose = false);

    // Retrieves a double array serialized as a JSON string from the environment variables
    std::optional<std::vector<double>> getDoubleArr(const char* key, bool verbose = false);

    // Retrieves a thread-local status code used by the wayfinder environment utility to report errors
    // These map directly to WFStatus codes. NOTE: the value in getError is invalidated by any call to any function in the
    // `wf::env` namespace other than `wf::env::getError()`, so getError should be called IMMEDIATELY after any
    // failed environment operation
    std::uint32_t getError();

    // Manually clears error
    void clearError();
}