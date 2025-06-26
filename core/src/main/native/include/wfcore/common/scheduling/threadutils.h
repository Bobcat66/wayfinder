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

#include <vector>
#include <thread>

namespace wf {

    enum class ThreadPriority {
        LOWEST,
        LOW,
        NORMAL,
        HIGH,
        HIGHEST,
        SENSITIVE,
        CRITICAL,
        REALTIME,
        ELEVEN
    };

    // Sets a thread to only run on certain CPU cores
    int setCPUAffinity(const std::thread& thread, const std::vector<int>& cores);

    // Sets thread priority of a thread
    int setThreadPriority(const std::thread& thread, ThreadPriority priority);

    // Returns the CPUs that a given thread is allowed to run on
    std::vector<int> getCPUAffinity(const std::thread& thread);
}