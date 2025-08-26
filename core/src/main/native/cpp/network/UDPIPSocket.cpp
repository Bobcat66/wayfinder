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

#include "wfcore/network/UDPIPSocket.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include "wfcore/common/wfexcept.h"

namespace wf {
    UDPIPSocket::UDPIPSocket(uint16_t port, const char* local_addr) 
    : BSDSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) {
        struct sockaddr_in local_addr_struct{};
        local_addr_struct.sin_family = AF_INET;
        local_addr_struct.sin_port = htons(port);
        if (local_addr) {
            in_addr local_ip{0};
            if (!inet_aton(local_addr, &local_ip)) {
                throw wf_status_error(WFStatus::NETWORK_BAD_ADDR,"Failed to set local IP address {}", local_addr);
            }
            local_addr_struct.sin_addr = local_ip;
        } else {
            local_addr_struct.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        if (bind(sockfd, reinterpret_cast<const struct sockaddr*>(&local_addr_struct), sizeof(local_addr_struct)) < 0) {
            close(sockfd);
            throw wf_status_error(
                WFStatus::NETWORK_BAD_SOCK,
                "Failed to bind UDP/IP socket to port {}: {}",port,strerror(errno)
            );
        }
    }
}