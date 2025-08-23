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

#include "wfcore/network/UDPIP_Sock.h"
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "wfcore/common/wfexcept.h"
#include "wfcore/common/logging.h"

namespace wf {
    UDPIP_Sock::UDPIP_Sock(uint16_t port, const char* addr) {
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
            throw wf_status_error(WFStatus::NETWORK_BAD_SOCK,"Failed to acquire UDP socket: {}", strerror(errno));
        }
        struct sockaddr_in local_addr{};
        local_addr.sin_family = AF_INET;
        local_addr.sin_port = htons(port);
        if (addr) {
            in_addr local_ip{0};
            if (!inet_aton(addr, &local_ip)) {
                throw wf_status_error(WFStatus::NETWORK_BAD_ADDR,"Failed to set local IP address {}", addr);
            }
            local_addr.sin_addr = local_ip;
        } else {
            local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        if (bind(sockfd, reinterpret_cast<const struct sockaddr*>(&local_addr), sizeof(local_addr)) < 0) {
            close(sockfd);
            throw wf_status_error(
                WFStatus::NETWORK_BAD_SOCK,
                "Failed to bind UDP socket to port {}: {}",port,strerror(errno)
            );
        }
    }

    UDPIP_Sock::~UDPIP_Sock() {
        close(sockfd);
    }


    WFStatusResult UDPIP_Sock::Send(const struct sockaddr_in* addr, socklen_t addrlen, void* buf, size_t len, int flags) {
        if (sendto(sockfd, buf, len, flags,reinterpret_cast<const struct sockaddr*>(addr), addrlen) < 0){
            sockerr = errno;
            if  (sockerr == EWOULDBLOCK || sockerr == EAGAIN) {
                // Socket would block, this is semi-nominal, return a lightweight WFStatusResult with no message
                return WFStatusResult::failure(WFStatus::NETWORK_WAITING);
            }
            return WFStatusResult::failure(WFStatus::NETWORK_BAD_SEND,strerror(sockerr));
        }
        sockerr = 0;
        return WFStatusResult::success();
    }

    WFStatusResult UDPIP_Sock::Recv(struct sockaddr_in* addr, socklen_t* addrlen, void* buf, size_t len, int flags) {
        if (recvfrom(sockfd, buf, len, flags, reinterpret_cast<struct sockaddr*>(addr), addrlen) < 0) {
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


    WFStatusResult UDPIP_Sock::SendMsg(const struct msghdr* msg, int flags) {
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
    WFStatusResult UDPIP_Sock::RecvMsg(struct msghdr* msg, int flags) {
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

    WFStatusResult UDPIP_Sock::SetSockOpt(int level, int option, const void* optval, socklen_t optlen) {
        if (setsockopt(sockfd, level, option, optval, optlen) < 0) {
            sockerr = errno;
            return WFStatusResult::failure(WFStatus::NETWORK_BAD_SOCKOPT,strerror(sockerr));
        }
        sockerr = 0;
        return WFStatusResult::success();
    }
    WFStatusResult UDPIP_Sock::GetSockOpt(int level, int option, void* optval, socklen_t* optlen) {
        if (getsockopt(sockfd, level, option, optval, optlen) < 0) {
            sockerr = errno;
            return WFStatusResult::failure(WFStatus::NETWORK_BAD_SOCKOPT,strerror(sockerr));
        }
        sockerr = 0;
        return WFStatusResult::success();
    }
}