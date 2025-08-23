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

#include <cstdint>
#include <sys/socket.h>
#include "wfcore/common/status.h"
#include <cstddef>
#include <netinet/in.h>

namespace wf {

    // a UDP/IP socket using the Berkeley sockets API, and wraps it in an RAII wrapper
    class UDPIP_Sock {
    public:
        UDPIP_Sock(uint16_t port, const char* addr = nullptr);
        ~UDPIP_Sock();
        // returns raw socket file descriptor
        int GetFD() { return sockfd; }
        // returns the actual errno, for richer programmatic error handling beyond the WFStatusResults
        int GetErrno() { return sockerr; }
        //send and recv return simple WFstatus codes, errno should be checked for more descriptive error messages
        WFStatusResult Send(const struct sockaddr_in* addr, socklen_t addrlen, void* buf, size_t len, int flags = 0);
        WFStatusResult Recv(struct sockaddr_in* addr, socklen_t* addrlen, void* buf, size_t len, int flags = 0);
        WFStatusResult SendMsg(const struct msghdr* msg, int flags = 0);
        WFStatusResult RecvMsg(struct msghdr* msg, int flags = 0);
        WFStatusResult SetSockOpt(int level, int option, const void* optval, socklen_t optlen);
        WFStatusResult GetSockOpt(int level, int option, void* optval, socklen_t* optlen);
    private:
        // socket file descriptor
        int sockfd;
        int sockerr;
    };
}