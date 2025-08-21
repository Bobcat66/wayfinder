/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2025 Jesse Kane, Valentina Carcassi
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

#include <mutex>

// header for utility functions to control the daemon process
namespace wfd {
    std::lock_guard<std::mutex> getLock();
    // Kills the wayfinder process with exit code 
    void shutdown();
    void reload();
    void restart();
    void reboot();
    void clearReqstore();
    bool shutdownRequested();
    bool reloadRequested();
    bool restartRequested();
    bool rebootRequested();
}