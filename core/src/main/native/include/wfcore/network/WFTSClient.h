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
#include "wfcore/network/Socket.h"
#include "wfcore/utils/FiniteStateMachine.h"
#include <thread>
#include <memory>
#include <netinet/in.h>
#include <arpa/inet.h>

#define WFTS_CLIENT_PORT 30001

namespace wf {
    
    class WFTSClient {
    public:
        WFTSClient(std::unique_ptr<Socket> sock_, void (*masterOffsetConsumer_)(int64_t));
        ~WFTSClient();
        void start();
        void stop();
    private:
        // Returns the offset between the PHC and wpi::Now in microseconds
        int64_t getOffset();
        void pingpong();
        void cleanup();
        // a function pointer to a function which consumes calculated offsets
        void (*masterOffsetConsumer)(int64_t);
        std::jthread worker;
        struct sockaddr_in servaddr;
        socklen_t servaddr_len;
        std::unique_ptr<Socket> sock;
        // Hardware capabilities
        unsigned int tsopts = 0;
        int phcfd; // PTP hardware clock file descriptor
        void* phc_caps;
        void* fsmClosure;
        std::unique_ptr<FiniteStateMachine> stateMachine;
    };
}