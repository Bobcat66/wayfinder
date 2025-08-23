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

#include "wfcore/network/TimeClient.h"
#include "wfcore/common/wfexcept.h"
#include "wips/timesync_packet.wips.h"
#include "wfcore/common/logging.h"
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/net_tstamp.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <net/if.h>

#define WFTS_MSG_LEADER     0b00000001 // Whether or not the message is from the leader
#define WFTS_MSG_BROADCAST  0b00000010 // Whether or not the message is broadcasted
#define WFTS_MSG_CRITICAL   0b00000100 // Whether or not the message is time-critical

#define WFTS_CLIENT_PORT 30001
#define WFTS_SERVER_PORT 30001

#define WFTS_TSPACKET_SIZE 13 // This is the size of the *packed* packet, the WIPS struct is padded

namespace wf {
    TimeClient::TimeClient(const char* broadcast_ip, const char* unicast_ip) : sock(WFTS_CLIENT_PORT) {

        // Poll hardware timestamping capabilities
        const char* iface = "eth0"; // Wayfinder always uses the eth0 interface for network communications
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name)-1);

        struct ethtool_ts_info tsi;
        memset(&tsi, 0, sizeof(tsi));
        tsi.cmd = ETHTOOL_GET_TS_INFO;
        ifr.ifr_data = (char*)&tsi;

        if (ioctl(sock.GetFD(), SIOCETHTOOL, &ifr) < 0) {
            // TODO: Special error code for this?
            throw wf_status_error(WFStatus::NETWORK_UNKNOWN, "Failed to retrieve network hardware information: {}", strerror(errno));
        }

        // construct tsopts bitfield
        if (tsi.so_timestamping & SOF_TIMESTAMPING_TX_HARDWARE) {
            globalLogger()->info("SOF_TIMESTAMPING_TX_HARDWARE support detected");
            tsopts |= SOF_TIMESTAMPING_TX_HARDWARE;
        } else {
            globalLogger()->info("SOF_TIMESTAMPING_TX_HARDWARE support not detected, falling back to SOF_TIMESTAMPING_TX_SOFTWARE");
            tsopts |= SOF_TIMESTAMPING_TX_SOFTWARE;
        }

        if (tsi.so_timestamping & SOF_TIMESTAMPING_RX_HARDWARE) {
            globalLogger()->info("SOF_TIMESTAMPING_RX_HARDWARE support detected");
            tsopts |= SOF_TIMESTAMPING_RX_HARDWARE;
        } else {
            globalLogger()->info("SOF_TIMESTAMPING_RX_HARDWARE support not detected, falling back to SOF_TIMESTAMPING_RX_SOFTWARE");
            tsopts |= SOF_TIMESTAMPING_RX_SOFTWARE;
        }

        // Apply tsopts to socket
        auto tsres = sock.SetSockOpt(SOL_SOCKET, SO_TIMESTAMPING, &tsopts, sizeof(tsopts));
        if (!tsres) {
            throw wf_result_error(tsres);
        }


        int enable = 1;
        // Enable broadcast support on the socket
        auto broadcast_res = sock.SetSockOpt(SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));
        if (!broadcast_res) {
            throw wf_result_error(broadcast_res);
        }


        // Configure unicast address
        struct in_addr ucip{0};
        if (!inet_aton(unicast_ip,&ucip)) {
            throw wf_status_error(WFStatus::NETWORK_BAD_ADDR,"TimeClient: Failed to set Unicast IP Address '{}'",unicast_ip);
        }
        uc_addr.sin_port = htons(WFTS_SERVER_PORT);
        uc_addr.sin_family = AF_INET;
        uc_addr.sin_addr = ucip;

        // Configure broadcast address
        struct in_addr bcip{0};
        if (!inet_aton(broadcast_ip,&bcip)) {
            throw wf_status_error(WFStatus::NETWORK_BAD_ADDR,"TimeClient: Failed to set Broadcast IP Address '{}'",broadcast_ip);
        }
        bc_addr.sin_port = htons(WFTS_SERVER_PORT);
        bc_addr.sin_family = AF_INET;
        bc_addr.sin_addr = bcip;
    }

    void TimeClient::pingpong() {

        // Step 1: Wait for the RIO to broadcast a sync message on the subnet broadcast address

        // msghdr construction
        char buffer[WFTS_TSPACKET_SIZE];
        char control[512];   // buffer for ancillary data

        struct msghdr msg;
        struct iovec iov;

        iov.iov_base = buffer;
        iov.iov_len = WFTS_TSPACKET_SIZE;

        msg.msg_name = &bc_addr;
        msg.msg_namelen = sizeof(bc_addr);
        msg.msg_iov = &iov;
        msg.msg_control = control;
        msg.msg_controllen = 512;
        msg.msg_flags = 0;


        auto res = sock.RecvMsg(
            &bc_addr,
            &bcaddrlen,
            buffer,
            sizeof(wips_timesync_packet_t)
        );
        if (!res) {
            // the socket is configured to block, it'll never return NETWORK_WAITING
            globalLogger()->error(res.what());
            return;
        }
    }
    TimeClient::~TimeClient() {

    }
}