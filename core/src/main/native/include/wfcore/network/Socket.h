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

#include <optional>
#include <sys/socket.h>
#include "wfcore/common/status.h"

namespace wf {
    class Socket {
    public:
        // Returns the raw file descriptor of the socket, if applicable
        virtual std::optional<int> getfd() = 0;
        virtual int geterr() = 0;
        virtual WFStatusResult SendMsg(const struct msghdr* msg, int flags = 0) = 0;
        virtual WFStatusResult RecvMsg(struct msghdr* msg, int flags = 0) = 0;
        virtual WFStatusResult SetSockOpt(int level, int option, const void* optval, socklen_t optlen) = 0;
        virtual WFStatusResult GetSockOpt(int level, int option, void* optval, socklen_t* optlen) = 0;
        virtual ~Socket();
        WFStatusResult SendTo(const void* buf, size_t len, int flags = 0, const struct sockaddr* in = NULL, socklen_t addrlen = 0) {
            struct msghdr msg;
            struct iovec iov;

            iov.iov_base = const_cast<void*>(buf);
            iov.iov_len = len;

            msg.msg_name = const_cast<struct sockaddr*>(in);
            msg.msg_namelen = addrlen;
            msg.msg_control = NULL;
            msg.msg_controllen = 0;
            msg.msg_iov = &iov;
            msg.msg_iovlen = 1;
            msg.msg_flags = 0;
            
            return SendMsg(&msg, flags);
        }
        WFStatusResult RecvFrom(void* buf, size_t len, int flags = 0, struct sockaddr* in = NULL, socklen_t* addrlen = NULL) {
            struct msghdr msg;
            struct iovec iov;

            iov.iov_base = buf;
            iov.iov_len = len;

            msg.msg_name = in;
            msg.msg_namelen = (addrlen) ? *addrlen : 0;
            msg.msg_control = NULL;
            msg.msg_controllen = 0;
            msg.msg_iov = &iov;
            msg.msg_iovlen = 1;
            msg.msg_flags = 0;

            auto res = RecvMsg(&msg, flags);

            if (addrlen) {
                *addrlen = msg.msg_namelen;
            }

            return res;
        }
        WFStatusResult Send(void* buf, size_t len, int flags = 0) {
            return SendTo(buf, len, flags, NULL, 0);
        }
        WFStatusResult Recv(void* buf, size_t len, int flags = 0) {
            return RecvFrom(buf, len, flags, NULL, NULL);
        }
    };
}