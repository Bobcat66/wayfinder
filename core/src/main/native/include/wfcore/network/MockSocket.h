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

namespace wf {
    class MockTimeSocket : public Socket {
    public:
        std::optional<int> getfd() override; 
        int geterr() override { return 0; };
        WFStatusResult SendMsg(const struct msghdr* msg, int flags = 0) override;
        WFStatusResult RecvMsg(struct msghdr* msg, int flags = 0) override;
        WFStatusResult SetSockOpt(int level, int option, const void* optval, socklen_t optlen) override;
        WFStatusResult GetSockOpt(int level, int option, void* optval, socklen_t* optlen) override;
    private:
        int opts = 0;
    }
}