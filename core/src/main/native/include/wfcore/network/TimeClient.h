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

#include <atomic>
#include "wfcore/network/UDPIP_Sock.h"
#include <thread>
#include <memory>

namespace wf {
    class TimeClient {
    public:
        // In general, broadcast_ip should be 10.TE.AM.255, and unicast_ip should be 10.TE.AM.2
        TimeClient(const char* broadcast_ip, const char* unicast_ip);
        ~TimeClient();
        void start();
        void stop();
        int64_t getNow();
        int64_t getOffset();
    private:
        // Returns the offset between the PHC and wpi::Now in microseconds
        int64_t getOffset();
        void listen();
        void pingpong();
        std::jthread worker;
        struct sockaddr_in servaddr;
        socklen_t servaddr_len;
        UDPIP_Sock sock;
        std::atomic_int64_t masterOffset{0};
        // Hardware capabilities
        unsigned int tsopts = 0;
        int phcfd; // PTP hardware clock file descriptor
        void* phc_caps;
    };
}