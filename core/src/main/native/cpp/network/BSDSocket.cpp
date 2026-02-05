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

#include "wfcore/network/BSDSocket.h"
#include "wfcore/common/wfexcept.h"
#include <poll.h>

namespace wf {
    BSDSocket::BSDSocket(int socket_family, int socket_type, int protocol) {
        if ((sockfd = socket(socket_family, socket_type, protocol)) < 0) {
            throw wf_status_error(WFStatus::NETWORK_BAD_SOCK,"Failed to acquire socket: {}", strerror(errno));
        }
    }

    BSDSocket::~BSDSocket() {
        close(sockfd);
    }

    WFStatusResult BSDSocket::SendMsg(const struct msghdr* msg, int flags) {
        if (sendmsg(sockfd, msg, flags) < 0) {
            sockerr = errno;
            if (sockerr == EWOULDBLOCK || sockerr == EAGAIN) {
                // Socket would block, this is semi-nominal, return a lightweight WFStatusResult with no message
                return WFStatusResult::failure(WFStatus::NETWORK_WAITING);
            }
            return WFStatusResult::failure(WFStatus::NETWORK_BAD_SEND,strerror(errno));
        }
        sockerr = 0;
        return WFStatusResult::success();
    }

    WFStatusResult BSDSocket::RecvMsg(struct msghdr* msg, int flags) {
        if (recvmsg(sockfd, msg, flags) < 0) {
            sockerr = errno;
            if (sockerr == EWOULDBLOCK || sockerr == EAGAIN) {
                // Socket would block, this is semi-nominal, return a lightweight WFStatusResult with no message
                return WFStatusResult::failure(WFStatus::NETWORK_WAITING);
            }
            return WFStatusResult::failure(WFStatus::NETWORK_BAD_RECV,strerror(errno));
        }
        sockerr = 0;
        return WFStatusResult::success();
    }

    WFStatusResult BSDSocket::SetSockOpt(int level, int option, const void* optval, socklen_t optlen) {
        if (setsockopt(sockfd, level, option, optval, optlen) < 0) {
            sockerr = errno;
            return WFStatusResult::failure(WFStatus::NETWORK_BAD_SOCKOPT,strerror(sockerr));
        }
        sockerr = 0;
        return WFStatusResult::success();
    }

    WFStatusResult BSDSocket::GetSockOpt(int level, int option, void* optval, socklen_t* optlen) {
        if (getsockopt(sockfd, level, option, optval, optlen) < 0) {
            sockerr = errno;
            return WFStatusResult::failure(WFStatus::NETWORK_BAD_SOCKOPT,strerror(sockerr));
        }
        sockerr = 0;
        return WFStatusResult::success();
    }

    WFResult<short> BSDSocket::Poll(short events,uint32_t timeout_ms) {
        struct pollfd pfd {
            .fd = sockfd,
            .events = events,
            .revents = 0
        };
        int ret = poll(&pfd, 1, timeout_ms);
        if (ret < 0) {
            // Error
            sockerr = errno;
            return WFResult<short>::failure(WFStatus::POSIX_ERROR, strerror(sockerr));
        } else if (ret == 0) {
            // Timed out
            sockerr = 0;
            return WFResult<short>::failure(WFStatus::NETWORK_WAITING);
        } else {
            return pfd.revents;
        }
    }
}