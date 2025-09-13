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

#include "wfcore/network/Socket.h"
#include <memory>
#include "wfcore/utils/FiniteStateMachine.h"
#include <thread>
#include <cstdint>

// A reference implementation of a two-stage WFTS Server. This is mainly provided for unit testing and is not actually used 
namespace wf {
    
    class WFTSServer {
    public:
        WFTSServer(std::unique_ptr<Socket> sock_, int64_t (*timesource_)(void), const char* addr = "127.0.0.1");
        ~WFTSServer();
        void cleanup();
        void start();
        void stop();
        // calculates offset between server timesource and system clock
        int64_t getOffset() const;
    private:
        int64_t (*timesource)(void);
        std::jthread worker;
        std::unique_ptr<FiniteStateMachine> server_fsm;
        void* server_closure;
        std::unique_ptr<Socket> sock;
    };
}