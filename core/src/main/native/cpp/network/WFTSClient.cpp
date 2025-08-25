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

#include "wfcore/network/WFTSClient.h"
#include "wfcore/common/wfexcept.h"
#include "wips/timesync_packet.wips.h"
#include "wfcore/common/logging.h"
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/net_tstamp.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <linux/ptp_clock.h>
#include <net/if.h>
#include <fcntl.h>
#include <unistd.h>
#include <wpi/timestamp.h>
#include <chrono>

// Flags

#define WFTS_MSG_LEADER             0b00000001 // Whether or not the message is from the leader
#define WFTS_MSG_BROADCAST          0b00000010 // Whether or not the message is broadcasted
#define WFTS_MSG_CRITICAL           0b00000100 // Whether or not the message is time-critical
#define WFTS_MSG_HASTIME            0b00001000 // Whether or not the message has a meaningful timestamp
#define WFTS_MSG_ERROR              0b10000000 // Error flag

// Flag combos for different types of messages

#define WFTS_MSG_SYNCFLAGS          (WFTS_MSG_LEADER | WFTS_MSG_BROADCAST | WFTS_MSG_CRITICAL)    // Required flags for a sync message
#define WFTS_MSG_FOLLOWUPFLAGS      (WFTS_MSG_LEADER | WFTS_MSG_BROADCAST | WFTS_MSG_HASTIME)     // Required flags for a followup message
#define WFTS_MSG_FOLLOWUPNFLAGS     (WFTS_MSG_CRITICAL)                                           // Disallowed flags for a followup message
#define WFTS_MSG_DELAYREQFLAGS      (WFTS_MSG_CRITICAL)                                           // Required flags for a delayreq message
#define WFTS_MSG_DELAYREQNFLAGS     (WFTS_MSG_LEADER | WFTS_MSG_BROADCAST | WFTS_MSG_HASTIME)     // Disallowed flags for a delayreq message
#define WFTS_MSG_DELAYRESPFLAGS     (WFTS_MSG_LEADER | WFTS_MSG_HASTIME)                          // Required flags for a delayresp message
#define WFTS_MSG_DELAYRESPNFLAGS    (WFTS_MSG_BROADCAST | WFTS_MSG_CRITICAL)                      // Disallowed flags for a delayresp message

#define WFTS_CLIENT_PORT 30001
#define WFTS_SERVER_PORT 30001

#define WFTS_TSPACKET_SIZE 13 // This is the size of the *packed* packet, the WIPS struct is padded

#define WFTS_PHC_SAMPLES 5 // the number of samples to take when synchronizing the PHC to the system clock

#define WFTS_TIMEOUT_US 40000 // The number of microseconds the socket will wait for a packet before timing out

namespace impl {
    // everything is in microseconds
    struct PingPongTimes {
        int64_t t0; // Master clock time when it sends sync
        int64_t t1; // Slave clock time when it receives sync
        int64_t t2; // Slave clock time when it sends delay_req
        int64_t t3; // Master clock time when it receives delay_req
    };
    // Below is a short derivation of the IEEE-1588 Algorithm,
    // so its easy to see why all of this makes sense:
    // offset + delay = t1 - t0
    // -offset + delay = t3 - t2
    // t1 - t0 - delay = t2 - t3 + delay = offset
    // t1 - t0 - delay + t2 - t3 + delay = 2 * offset
    // t1 - t0 - t3 + t2 = 2 * offset


    // returns the offset of the system clock relative to wpi::Now()
    // defined as sys(t) - wpi(t)
    int64_t wpi_sys_offset() {
        auto t0 = std::chrono::system_clock::now();
        auto t1 = static_cast<int64_t>(wpi::Now());
        return std::chrono::duration_cast<std::chrono::microseconds>(
            t0.time_since_epoch()
        ).count() - t1;
    }

    bool flagcheck(uint8_t flags, uint8_t reqs, uint8_t disallowed = 0) {
        return ((flags & reqs) == reqs) && ((flags & disallowed) == 0);
    }

}

// NOTE: This code is particularly vulnerable to subtle sign-conversion logic errors.
// I'm 99% sure I did everything correctly, but just in case, this code should be
// audited if there are any otherwise unexplainable bugs in the future, especially regarding pose accuracy
// Especially the PHC offset calculation logic
namespace wf {
    static loggerPtr tcLogger = LoggerManager::getInstance().getLogger("WFTSClient",LogGroup::Network);
    // TODO: Maybe make WFTSClient a statusful object?
    WFTSClient::WFTSClient() 
    : sock(WFTS_CLIENT_PORT) , servaddr{0}, servaddr_len{sizeof(servaddr)} {

        // Poll hardware timestamping capabilities
        const char* iface = "eth0"; // Wayfinder always uses the eth0 interface for network communications
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name)-1);

        struct ethtool_ts_info tsi;
        memset(&tsi, 0, sizeof(tsi));
        tsi.cmd = ETHTOOL_GET_TS_INFO;
        ifr.ifr_data = reinterpret_cast<char*>(&tsi);

        if (ioctl(sock.GetFD(), SIOCETHTOOL, &ifr) < 0) {
            // TODO: Special error code for this?
            throw wf_status_error(WFStatus::NETWORK_UNKNOWN, "Failed to retrieve eth0 NIC hardware information: {}", strerror(errno));
        }

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = WFTS_TIMEOUT_US;

        auto rcvtimeo_res = sock.SetSockOpt(SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        if (!rcvtimeo_res) {
            throw wf_result_error(rcvtimeo_res);
        }
        auto sndtimeo_res = sock.SetSockOpt(SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        if (!sndtimeo_res) {
            throw wf_result_error(sndtimeo_res);
        }

        bool hwsupport;

        // construct tsopts bitfield
        if ((tsi.so_timestamping & SOF_TIMESTAMPING_TX_HARDWARE) && (tsi.so_timestamping & SOF_TIMESTAMPING_RX_HARDWARE)) {
            tcLogger->info("Hardware timestamping support detected");
            tsopts |= (SOF_TIMESTAMPING_TX_HARDWARE | SOF_TIMESTAMPING_RX_HARDWARE);
            hwsupport = true;
        } else {
            tcLogger->info("Hardware timestamping support not detected, falling back to software timestamping");
            tsopts |= (SOF_TIMESTAMPING_TX_SOFTWARE | SOF_TIMESTAMPING_RX_SOFTWARE);
            hwsupport = false;
        }

        tsopts |= (SOF_TIMESTAMPING_RAW_HARDWARE | SOF_TIMESTAMPING_SOFTWARE);

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

        // Open PHC file
        if (hwsupport) {
            if (tsi.phc_index < 0) {
                close(phcfd);
                throw wf_status_error(WFStatus::NETWORK_UNKNOWN, "NIC Reported hardware timestamping capabilities but does not have a valid PHC");
            }
            auto pathstr = std::format("/dev/ptp{}",tsi.phc_index);
            phcfd = open(pathstr.c_str(),O_RDONLY);
            if (phcfd < 0) {
                throw wf_status_error(WFStatus::BAD_ACQUIRE,"Failed to acquire PHC: {}",strerror(errno));
            }
            phc_caps = new ptp_clock_caps;
            if (ioctl(phcfd, PTP_CLOCK_GETCAPS, phc_caps) < 0) {
                delete static_cast<ptp_clock_caps*>(phc_caps);
                close(phcfd);
                throw wf_status_error(WFStatus::NETWORK_UNKNOWN,"Failed to query PHC capabilities: {}",strerror(errno));
            }
        } else {
            phc_caps = nullptr;
            phcfd = -1;
        }
    }

    WFTSClient::~WFTSClient() {
        if (phcfd >= 0) { close(phcfd); }
        if (phc_caps) { delete static_cast<ptp_clock_caps*>(phc_caps); }
    }

    void WFTSClient::start() {
        if (worker.joinable()) return;
        worker = std::jthread([this](std::stop_token st){
            while (!st.stop_requested()) {
                this->pingpong();
            }
        });
    }

    void WFTSClient::stop() {
        if (worker.joinable()) {
            worker.request_stop();
            worker.join();  // blocks until the worker finishes
        }
    }

    // TODO: Maybe rewrite this as an actual state machine?
    void WFTSClient::pingpong() {

        auto offset = getOffset();
        impl::PingPongTimes timestamps;

        // Step 1: Wait for the RIO to broadcast a sync message on the subnet broadcast address, get t0 and t1

        // prepare message
        char sync_payload[WFTS_TSPACKET_SIZE];
        char sync_control[512];
        
        struct msghdr sync_msg{0};
        struct iovec sync_iov{0};
        
        sync_iov.iov_base = sync_payload;
        sync_iov.iov_len = sizeof(sync_payload);
    
        sync_msg.msg_name = &servaddr;
        sync_msg.msg_namelen = servaddr_len;
        sync_msg.msg_iov = &sync_iov;
        sync_msg.msg_iovlen = 1;
        sync_msg.msg_control = sync_control;
        sync_msg.msg_controllen = sizeof(sync_control);

        // Poll socket, block until message is received
        auto sync_res = sock.RecvMsg(&sync_msg);
        if (!sync_res) {
            // the socket is configured to block, it'll never return NETWORK_WAITING
            tcLogger->error(sync_res.what());
            return;
        }

        // Decode message
        auto sync_blob = wips_blob_stackwrap(reinterpret_cast<unsigned char*>(sync_payload), sizeof(sync_payload));
        wips_timesync_packet_t sync_packet;
        memset(&sync_packet, 0, sizeof(sync_packet));
        auto sync_decodeRes = wips_decode_timesync_packet(&sync_packet, &sync_blob);
        if (sync_decodeRes.status_code != WIPS_STATUS_OK) {
            tcLogger->error("Failed to decode sync message");
            return;
        }
        if (!impl::flagcheck(sync_packet.flags, WFTS_MSG_SYNCFLAGS)) {
            // Packet does not have expected flags, discard it
            tcLogger->error("Sync message {} did not have expected flags set.",sync_packet.packet_id);
            return;
        }
        if (sync_packet.flags & WFTS_MSG_ERROR) {
            tcLogger->error("Sync message {} had the error flag set",sync_packet.packet_id);
            return;
        }
        servaddr_len = sync_msg.msg_namelen;
        uint32_t lastPacketID = sync_packet.packet_id;

        // Parse ancillary data (rx timestamp)
        struct cmsghdr* sync_cmsg;
        // loop through all control messages in socket response
        for (sync_cmsg = CMSG_FIRSTHDR(&sync_msg); sync_cmsg != NULL; sync_cmsg = CMSG_NXTHDR(&sync_msg, sync_cmsg)) {
            if (sync_cmsg->cmsg_type == SO_TIMESTAMPING) {
                struct timespec* ts = reinterpret_cast<struct timespec*>(CMSG_DATA(sync_cmsg));
                // pmo stands for "pointer memory offset", and nothing else
                int pmo = (tsopts & SOF_TIMESTAMPING_RX_HARDWARE) ? 2 : 0;
                timestamps.t1 = (ts[pmo].tv_sec * 1e6) + (ts[pmo].tv_nsec/1000) - offset;
                break;
            }
        }

        if (sync_packet.flags & WFTS_MSG_HASTIME) {
            // One-stage synchronization, use sync_packet's timestamp as t0 directly
            timestamps.t0 = sync_packet.timestamp;
        } else {
            // Two-stage synchronization, wait for followup message with t0
            char followup_buffer[WFTS_TSPACKET_SIZE];

            auto followup_res = sock.Recv(&servaddr, &servaddr_len, followup_buffer, WFTS_TSPACKET_SIZE);
            if (!followup_res) {
                // the socket is configured to block, it'll never return NETWORK_WAITING
                tcLogger->error(followup_res.what());
                return;
            }

            // Decode message
            auto followup_blob = wips_blob_stackwrap(reinterpret_cast<unsigned char*>(followup_buffer),WFTS_TSPACKET_SIZE);
            wips_timesync_packet_t followup_packet;
            memset(&followup_packet, 0, sizeof(followup_packet));
            auto followup_decodeRes = wips_decode_timesync_packet(&followup_packet, &followup_blob);
            if (followup_decodeRes.status_code != WIPS_STATUS_OK) {
                tcLogger->error("Failed to decode followup message");
                return;
            }

            if (followup_packet.packet_id != (lastPacketID + 1)) {
                tcLogger->error("Followup message {} did not have the expected packet ID {}", followup_packet.packet_id, (lastPacketID + 1));
                return;
            }

            if (!impl::flagcheck(followup_packet.flags, WFTS_MSG_FOLLOWUPFLAGS, WFTS_MSG_FOLLOWUPNFLAGS)) {
                // Packet does not have expected flags, discard it
                tcLogger->error("Followup message {} did not have expected flags set.", followup_packet.packet_id);
                return;
            }

            if (followup_packet.flags & WFTS_MSG_ERROR) {
                tcLogger->error("Followup message {} had error flag set", followup_packet.packet_id);
                return;
            }

            timestamps.t0 = followup_packet.timestamp;

            lastPacketID++;

        }

        // Step 2: Send Delayreq message, get t1_follower
        
        // Prepare packet
        wips_timesync_packet_t delayreq_packet;
        delayreq_packet.flags = WFTS_MSG_DELAYREQFLAGS;
        // Delayreq packets have the same packet ID as the most recent broadcast received by the client + 1
        delayreq_packet.packet_id = lastPacketID + 1;
        delayreq_packet.timestamp = 0;

        char delayreq_payload[WFTS_TSPACKET_SIZE];

        // Prepare delayreq send message
        struct msghdr delayreq_msg{0};
        struct iovec delayreq_iov{0};

        delayreq_iov.iov_base = delayreq_payload;
        delayreq_iov.iov_len = sizeof(delayreq_payload);

        delayreq_msg.msg_name = &servaddr;
        delayreq_msg.msg_namelen = servaddr_len;
        delayreq_msg.msg_iov = &delayreq_iov;
        delayreq_msg.msg_iovlen = 1;
        delayreq_msg.msg_control = NULL;
        delayreq_msg.msg_controllen = 0;

        auto delayreq_blob = wips_blob_stackwrap(
            reinterpret_cast<unsigned char*>(delayreq_payload),
            sizeof(delayreq_payload)
        );

        auto delayreq_encodeResult = wips_encode_nrb_timesync_packet(&delayreq_blob, &delayreq_packet);
        if (delayreq_encodeResult.status_code != WIPS_STATUS_OK) {
            tcLogger->error("Failed to encode delayreq message");
            return;
        }

        // Send message over unicast UDP to server
        auto delayreq_res = sock.SendMsg(&delayreq_msg);
        if (!delayreq_res) {
            tcLogger->error(delayreq_res.what());
            return;
        }

        lastPacketID++;

        // By this point, the kernel has stored the transmission time of the delayreq msg in the error queue
        // To retrieve it, we need to send a recv request to the socket with the MSG_ERRQUEUE flag
        char delayreq_control[512];

        struct msghdr delayreq_ctrl_msg{0};
        struct iovec delayreq_ctrl_iov = { nullptr, 0 }; // dummy iovec
        
        delayreq_ctrl_msg.msg_name = NULL;
        delayreq_ctrl_msg.msg_namelen = 0;
        delayreq_ctrl_msg.msg_iov = &delayreq_ctrl_iov;
        delayreq_ctrl_msg.msg_iovlen = 1;
        delayreq_ctrl_msg.msg_control = delayreq_control;
        delayreq_ctrl_msg.msg_controllen = sizeof(delayreq_control);

        auto delayreq_ctrl_res = sock.RecvMsg(&delayreq_ctrl_msg,MSG_ERRQUEUE);
        if (!delayreq_ctrl_res) {
            tcLogger->error(delayreq_ctrl_res.what());
            return;
        }

        // Parse ancillary data (tx timestamp)
        struct cmsghdr* delayreq_cmsg;
        // loop through all control messages in socket response
        for (delayreq_cmsg = CMSG_FIRSTHDR(&delayreq_ctrl_msg); delayreq_cmsg != NULL; delayreq_cmsg = CMSG_NXTHDR(&delayreq_ctrl_msg, delayreq_cmsg)) {
            if (delayreq_cmsg->cmsg_type == SO_TIMESTAMPING) {
                struct timespec* ts = reinterpret_cast<struct timespec*>(CMSG_DATA(delayreq_cmsg));
                // pmo stands for "pointer memory offset", and nothing else
                int pmo = (tsopts & SOF_TIMESTAMPING_TX_HARDWARE) ? 2 : 0;
                timestamps.t2 = (ts[pmo].tv_sec * 1e6) + (ts[pmo].tv_nsec/1000) - offset;
                break;
            }
        }

        // Step 3: Await Delayresp message from the RIO, retrieve t3
        // TODO: We might be able to use Recv instead of RecvMsg

        // Prepare message
        char delayresp_payload[WFTS_TSPACKET_SIZE];

        auto delayresp_res = sock.Recv(&servaddr, &servaddr_len, delayresp_payload, sizeof(delayresp_payload));
        if (!delayresp_res) {
            tcLogger->error(delayresp_res.what());
            return;
        }

        // Decode message
        auto delayresp_blob = wips_blob_stackwrap(
            reinterpret_cast<unsigned char*>(delayresp_payload),
            sizeof(delayresp_payload)
        );
        wips_timesync_packet_t delayresp_packet;
        auto delayresp_decodeRes = wips_decode_timesync_packet(&delayresp_packet, &delayresp_blob);
        if (delayresp_decodeRes.status_code != WIPS_STATUS_OK) {
            tcLogger->error("Failed to decode delayresp message");
            return;
        }

        // On unicast delayresp messages the server increments the packetID that the delayreq was sent with
        if (delayresp_packet.packet_id != lastPacketID + 1) {
            tcLogger->error("Delayresp {} did not have expected packet ID {}", delayresp_packet.packet_id, lastPacketID + 1);
            return;
        }

        // Check message
        if (!impl::flagcheck(delayresp_packet.flags, WFTS_MSG_DELAYRESPFLAGS, WFTS_MSG_DELAYRESPNFLAGS)) {
            tcLogger->error("Delayresp {} did not have expected flags", delayresp_packet.packet_id);
            return;
        }

        // Check for error
        if (delayresp_packet.flags & WFTS_MSG_ERROR) {
            tcLogger->error("Delayresp {} had error flag set", delayresp_packet.packet_id);
        }
        timestamps.t3 = delayresp_packet.timestamp;

        // Step 4: Compute offset
        auto masterOffset_tmp = (timestamps.t1 - timestamps.t0 - timestamps.t3 + timestamps.t2)/2;
        masterOffset.store(masterOffset_tmp);
    }

    // Either returns the offset between the PHC and wpi::now, or the system realtime clock and wpi::now
    int64_t WFTSClient::getOffset() {
        if (phcfd < 0) {
            // phcfd is negative, we are using system timestamps
            return impl::wpi_sys_offset();
        }
        auto phc_caps_ptr = static_cast<struct ptp_clock_caps*>(phc_caps);
        if (phc_caps_ptr->cross_timestamping) {
            // Hardware synchronization is supported, calculate offset with PTP_SYS_OFFSET_PRECISE
            struct ptp_sys_offset_precise hwoffset;
            if (ioctl(phcfd, PTP_SYS_OFFSET_PRECISE, &hwoffset) >= 0) {
                int64_t phcmicros = (hwoffset.device.sec * 1e6) + (hwoffset.device.nsec / 1000);
                int64_t sysmicros = (hwoffset.sys_realtime.sec * 1e6) + (hwoffset.sys_realtime.nsec / 1000);
                int64_t wpimicros = sysmicros + impl::wpi_sys_offset();
                return phcmicros - wpimicros;
            }
            tcLogger->warn("Failed to retrieve precise offset, falling back to PTP_SYS_OFFSET");
        }
        // Fallback to software
        struct ptp_sys_offset_extended swoffset;
        swoffset.n_samples = WFTS_PHC_SAMPLES;
        if (ioctl(phcfd, PTP_SYS_OFFSET_EXTENDED, &swoffset) < 0) {
            tcLogger->error("Failed to calculate offset between system clock and PHC");
            return impl::wpi_sys_offset();
        }
        int64_t accumulator = 0; // average offset between the PHC and SYSTEM clock, in nanoseconds
        for (int i = 0; i < WFTS_PHC_SAMPLES; ++i) {
            int64_t sys_t0 = (swoffset.ts[i][0].sec * 1e9) + swoffset.ts[i][0].nsec;
            int64_t phc_t1 = (swoffset.ts[i][1].sec * 1e9) + swoffset.ts[i][1].nsec;
            int64_t sys_t2 = (swoffset.ts[i][2].sec * 1e9) + swoffset.ts[i][2].nsec;
            int64_t delay = (sys_t2 - sys_t0)/2;
            int64_t sys_offset = sys_t0 - phc_t1 + delay;
            accumulator += sys_offset;   
        }
        double avgSysOffsetNs = static_cast<double>(accumulator) / WFTS_PHC_SAMPLES;
        int64_t avgWpiOffsetUs = -static_cast<int64_t>(avgSysOffsetNs / 1000.0) + impl::wpi_sys_offset();
        return avgWpiOffsetUs;
    }

    int64_t WFTSClient::getNow() {
        return wpi::Now() - masterOffset.load();
    }
    int64_t WFTSClient::getMasterOffset() {
        return masterOffset.load();
    }
}