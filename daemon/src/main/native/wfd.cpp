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

#include "wfd.h"
#include <atomic>
#include <cstdint>

namespace impl {
    static std::mutex daemon_mutex;
    static std::mutex reqs_mutex;
    enum class Request : uint8_t {
        NONE = 0x00,
        SHUTDOWN = 0x01,
        RELOAD = 0x02,
        RESTART = 0x03,
        REBOOT = 0x04
    };
    std::atomic<Request> reqstore(Request::NONE);
}

namespace wfd {
    std::lock_guard<std::mutex> getLock() {
        return std::lock_guard(impl::daemon_mutex);
    }
    void shutdown() {
        impl::reqstore.store(impl::Request::SHUTDOWN);
    }
    void reload() {
        impl::reqstore.store(impl::Request::RELOAD);
    }
    void restart() {
        impl::reqstore.store(impl::Request::RESTART);
    }
    void reboot() {
        impl::reqstore.store(impl::Request::REBOOT);
    }
    void clearReqstore() {
        impl::reqstore.store(impl::Request::NONE);
    }
    bool shutdownRequested() {
        return (impl::reqstore.load() == impl::Request::SHUTDOWN);
    }
    bool reloadRequested() {
        return (impl::reqstore.load() == impl::Request::RELOAD);
    }
    bool restartRequested() {
        return (impl::reqstore.load() == impl::Request::RESTART);
    }
    bool rebootRequested() {
        return (impl::reqstore.load() == impl::Request::REBOOT);
    }
}