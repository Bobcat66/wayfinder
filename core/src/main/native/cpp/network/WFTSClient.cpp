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
#include "wfdetail/wfts/wfts_defs.h"

#define WFTS_SERVER_PORT 30001
#define WFTS_PHC_SAMPLES 5 // the number of samples to take when synchronizing the PHC to the system clock
#define WFTS_TIMEOUT_US 500000 // The number of microseconds the socket will wait for a packet before timing out

#define WFTS_AWAIT_SYNC 0
#define WFTS_PROCESS_SYNC 1
#define WFTS_AWAIT_FOLLOWUP 2
#define WFTS_PROCESS_FOLLOWUP 3
#define WFTS_SEND_DELAYREQ 4
#define WFTS_PROCESS_DREQCTL 5
#define WFTS_AWAIT_DELAYRESP 6
#define WFTS_PROCESS_DELAYRESP 7
#define WFTS_COMPUTE_OFFSET 8

// TODO: Implement retries
namespace impl {
    using namespace wf;
    static loggerPtr tcLogger = LoggerManager::getInstance().getLogger("WFTSClient",LogGroup::Network);

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
    static int64_t wpi_sys_offset() {
        auto t0 = std::chrono::system_clock::now();
        auto t1 = static_cast<int64_t>(wpi::Now());
        return std::chrono::duration_cast<std::chrono::microseconds>(
            t0.time_since_epoch()
        ).count() - t1;
    }

    // Internal interface for WFTS State Machine callbacks
    struct wfts_fsm_closure {
        WFTSClient* client;
        int64_t (WFTSClient::*getOffset)();
        void (*masterOffsetConsumer)(int64_t);
        struct sockaddr_storage& servaddr;
        socklen_t& servaddr_len;
        std::unique_ptr<Socket>& sock;
        unsigned int& tsopts;
        PingPongTimes timestamps;
        uint32_t lastPacketID;
        int64_t offset;
        char controlBuf[512];
        char payloadBuf[WFTS_TSPACKET_SIZE];
    };

    static wfts_fsm_closure* getClosure(void* raw) {
        return static_cast<wfts_fsm_closure*>(raw);
    }

    void clearBuffers(wfts_fsm_closure* closure) {
        memset(closure->controlBuf, 0, sizeof(closure->controlBuf));
        memset(closure->payloadBuf, 0, sizeof(closure->payloadBuf));
    }

    // Resets closure, Awaits for SYNC from the server, stores it in closure buffers,
    // reads rx timestamp from control messages and sets t1, shifts to PROCESS_SYNC
    // Shifts to AWAIT_SYNC on failure
    uint32_t await_sync_impl(FSMInterface* iface, void* rawClosure) {
        WF_DEBUGLOG(tcLogger, "AWAITING SYNC");
        wfts_fsm_closure* closure = getClosure(rawClosure);
        clearBuffers(closure);
        memset(&(closure->timestamps), 0, sizeof(closure->timestamps));
        closure->offset = (closure->client->*(closure->getOffset))();
        struct msghdr msg;
        struct iovec iov;

        iov.iov_base = closure->payloadBuf;
        iov.iov_len = sizeof(closure->payloadBuf);

        msg.msg_name = &(closure->servaddr);
        msg.msg_namelen = closure->servaddr_len;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = closure->controlBuf;
        msg.msg_controllen = sizeof(closure->controlBuf);

        auto res = closure->sock->RecvMsg(&msg);
        closure->servaddr_len = msg.msg_namelen;

        if (!res) {
            tcLogger->error(res.what());
            return WFTS_AWAIT_SYNC;
        }
        // Parse ancillary data (rx timestamp)
        struct cmsghdr* cmsg;
        // loop through all control messages in socket response
        for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
            if (cmsg->cmsg_type == SCM_TIMESTAMPING) {
                struct timespec* ts = reinterpret_cast<struct timespec*>(CMSG_DATA(cmsg));
                // pmo stands for "pointer memory offset", and nothing else
                int pmo = (closure->tsopts & SOF_TIMESTAMPING_RX_HARDWARE) ? 2 : 0;
                closure->timestamps.t1 = (ts[pmo].tv_sec * 1e6) + (ts[pmo].tv_nsec/1000) - closure->offset;
                break;
            }
        }

        return WFTS_PROCESS_SYNC;
    }

    // Validates & processes SYNC, if SYNC has a time, sets t0 and shifts to SEND_DELAYREQ
    // Otherwise, shifts to AWAIT_FOLLOWUP
    // Shifts to AWAIT_SYNC on failure
    uint32_t process_sync_impl(FSMInterface* iface, void* rawClosure) {
        WF_DEBUGLOG(tcLogger, "SHIFTING TO PROCESS_SYNC");
        wfts_fsm_closure* closure = getClosure(rawClosure);

        // Decode packet
        auto blob = wips_blob_stackwrap(
            closure->payloadBuf,
            sizeof(closure->payloadBuf)
        );
        wips_timesync_packet_t packet;
        memset(&packet, 0, sizeof(packet));

        auto decodeRes = wips_decode_timesync_packet(&packet, &blob);
        if (decodeRes.status_code != WIPS_STATUS_OK) {
            tcLogger->error("Failed to decode sync message");
            return WFTS_AWAIT_SYNC;
        }

        // Check flags
        if (!detail::flagcheck(packet.flags, WFTS_MSG_SYNCFLAGS)) {
            // Packet does not have expected flags, discard it
            tcLogger->error("Sync message {} did not have expected flags set.",packet.packet_id);
            return WFTS_AWAIT_SYNC;
        }
        if (packet.flags & WFTS_MSG_ERROR) {
            tcLogger->error("Sync message {} had the error flag set",packet.packet_id);
            return WFTS_AWAIT_SYNC;
        }
        closure->lastPacketID = packet.packet_id;

        if (packet.flags & WFTS_MSG_HASTIME) {
            closure->timestamps.t0 = packet.timestamp;
            return WFTS_SEND_DELAYREQ;
        }
        return WFTS_AWAIT_FOLLOWUP;

    }

    // Awaits FOLLOWUP from server, stores result in closure, shifts to PROCESS_FOLLOWUP
    // shifts to AWAIT_SYNC on failure
    uint32_t await_followup_impl(FSMInterface* iface, void* rawClosure) {
        // This function is causing segmentation faults. Fix ASAP
        WF_DEBUGLOG(tcLogger, "SHIFTING TO AWAIT_FOLLOWUP");
        wfts_fsm_closure* closure = getClosure(rawClosure);
        clearBuffers(closure);
        //WF_DEBUGLOG(tcLogger, "{}", closure->servaddr_len);
        //WF_DEBUGLOG(tcLogger, "Cleared buffers");
        auto res = closure->sock->RecvFrom(
            closure->payloadBuf,
            sizeof(closure->payloadBuf), 
            0,
            reinterpret_cast<sockaddr*>(&(closure->servaddr)),
            &(closure->servaddr_len)
        );
        //WF_DEBUGLOG(tcLogger, "Received FOLLOWUP");

        if (!res) {
            tcLogger->error(res.what());
            return WFTS_AWAIT_SYNC;
        }
        return WFTS_PROCESS_FOLLOWUP;
    }

    // Validates and processes FOLLOWUP, sets t0 and shifts to SEND_DELAYREQ
    // Shifts to AWAIT_SYNC on failure
    uint32_t process_followup_impl(FSMInterface* iface, void* rawClosure) {
        WF_DEBUGLOG(tcLogger, "SHIFTING TO PROCESS_FOLLOWUP");
        wfts_fsm_closure* closure = getClosure(rawClosure);

         // Decode packet
        auto blob = wips_blob_stackwrap(
            closure->payloadBuf,
            sizeof(closure->payloadBuf)
        );
        wips_timesync_packet_t packet;
        memset(&packet, 0, sizeof(packet));

        auto decodeRes = wips_decode_timesync_packet(&packet, &blob);
        if (decodeRes.status_code != WIPS_STATUS_OK) {
            tcLogger->error("Failed to decode followup message");
            return WFTS_AWAIT_SYNC;
        }
        // Check packet ID
        if (packet.packet_id != (closure->lastPacketID + 1)) {
            tcLogger->error("Followup message {} did not have expected packet ID {}",packet.packet_id,(closure->lastPacketID + 1));
            return WFTS_AWAIT_SYNC;
        }

        // Check flags
        if (!detail::flagcheck(packet.flags, WFTS_MSG_FOLLOWUPFLAGS, WFTS_MSG_FOLLOWUPNFLAGS)) {
            // Packet does not have expected flags, discard it
            tcLogger->error("Followup message {} did not have expected flags set.",packet.packet_id);
            return WFTS_AWAIT_SYNC;
        }
        if (packet.flags & WFTS_MSG_ERROR) {
            tcLogger->error("Followup message {} had the error flag set",packet.packet_id);
            return WFTS_AWAIT_SYNC;
        }
        closure->lastPacketID = packet.packet_id;
        closure->timestamps.t0 = packet.timestamp;
        return WFTS_SEND_DELAYREQ;
    }

    // Encodes and sends DELAYREQ to server, stores data in closure, shifts to PROCESS_DREQCTL
    // Shifts to AWAIT_SYNC on failure 
    uint32_t send_delayreq_impl(FSMInterface* iface, void* rawClosure) {
        WF_DEBUGLOG(tcLogger, "SHIFTING TO SEND_DELAYREQ");
        wfts_fsm_closure* closure = getClosure(rawClosure);
        clearBuffers(closure);

        // Encode packet
        wips_timesync_packet_t packet;
        packet.flags = WFTS_MSG_DELAYREQFLAGS;
        packet.packet_id = (closure->lastPacketID + 1);
        packet.timestamp = 0;
        auto blob = wips_blob_stackwrap(closure->payloadBuf, sizeof(closure->payloadBuf));
        auto encodeRes = wips_encode_nrb_timesync_packet(&blob, &packet);
        if (encodeRes.status_code != WIPS_STATUS_OK) {
            tcLogger->error("Failed to encode delayreq message");
            return WFTS_AWAIT_SYNC;
        }

        // Prepare message
        struct msghdr msg;
        struct iovec iov;

        iov.iov_base = closure->payloadBuf;
        iov.iov_len = sizeof(closure->payloadBuf);

        msg.msg_name = &(closure->servaddr);
        msg.msg_namelen = closure->servaddr_len;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = nullptr;
        msg.msg_controllen = 0;

        // Send
        auto res = closure->sock->SendMsg(&msg);
        if (!res) {
            tcLogger->error(res.what());
            return WFTS_AWAIT_SYNC;
        }

        closure->lastPacketID++;
        return WFTS_PROCESS_DREQCTL;
    }

    // Reads control messages from socket Error Queue, processes them and sets t2, shifts to AWAIT_DELAYRESP
    // shifts to AWAIT_SYNC on failure
    uint32_t process_dreqctl_impl(FSMInterface* iface, void* rawClosure) {
        WF_DEBUGLOG(tcLogger, "SHIFTING TO PROCESS_DREQCTL");
        wfts_fsm_closure* closure = getClosure(rawClosure);
        clearBuffers(closure);

        struct msghdr msg;
        struct iovec iov = { nullptr, 0 }; // dummy iovec
        
        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = closure->controlBuf;
        msg.msg_controllen = sizeof(closure->controlBuf);

        auto res = closure->sock->RecvMsg(&msg,MSG_ERRQUEUE);
        if (!res) {
            tcLogger->error(res.what());
            return WFTS_AWAIT_SYNC;
        }

        // Parse ancillary data (tx timestamp)
        struct cmsghdr* cmsg;
        // loop through all control messages in socket response
        for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
            if (cmsg->cmsg_type == SCM_TIMESTAMPING) {
                WF_DEBUGLOG(tcLogger, "Timestamp retrieved from CTL");
                struct timespec* ts = reinterpret_cast<struct timespec*>(CMSG_DATA(cmsg));
                // pmo stands for "pointer memory offset", and nothing else
                int pmo = (closure->tsopts & SOF_TIMESTAMPING_TX_HARDWARE) ? 2 : 0;
                closure->timestamps.t2 = (ts[pmo].tv_sec * 1e6) + (ts[pmo].tv_nsec/1000) - closure->offset;
                break;
            }
        }

        return WFTS_AWAIT_DELAYRESP;
    }

    // Awaits DELAYRESP from server, stores DELAYRESP in closure, shifts to PROCESS_DELAYRESP
    // shifts to AWAIT_SYNC on failure
    uint32_t await_delayresp_impl(FSMInterface* iface, void* rawClosure) {
        WF_DEBUGLOG(tcLogger, "SHIFTING TO AWAIT_DELAYRESP");
        wfts_fsm_closure* closure = getClosure(rawClosure);
        clearBuffers(closure);
        
        auto res = closure->sock->RecvFrom(
            closure->payloadBuf, sizeof(closure->payloadBuf),
            0,
            reinterpret_cast<sockaddr*>(&(closure->servaddr)),
            &(closure->servaddr_len)
        );

        if (!res) {
            tcLogger->error(res.what());
            return WFTS_AWAIT_SYNC;
        }

        return WFTS_PROCESS_DELAYRESP;
    }

    // Processes DELAYRESP, sets t3, shifts to COMPUTE_OFFSET
    // shifts to AWAIT_SYNC on failure
    uint32_t process_delayresp_impl(FSMInterface* iface, void* rawClosure) {
        WF_DEBUGLOG(tcLogger, "SHIFTING TO PROCESS_DELAYRESP");
        wfts_fsm_closure* closure = getClosure(rawClosure);
        
        auto blob = wips_blob_stackwrap(closure->payloadBuf, sizeof(closure->payloadBuf));
        wips_timesync_packet_t packet;
        memset(&packet, 0, sizeof(packet));
        auto decodeRes = wips_decode_timesync_packet(&packet, &blob);
        if (decodeRes.status_code != WIPS_STATUS_OK) {
            tcLogger->error("Failed to decode delayresp message");
            return WFTS_AWAIT_SYNC;
        }

        if (packet.packet_id != (closure->lastPacketID + 1)) {
            tcLogger->error("Delayresp {} did not have expected packet ID {}", packet.packet_id, (closure->lastPacketID + 1));
            return WFTS_AWAIT_SYNC;
        }

        // Check message
        if (!detail::flagcheck(packet.flags, WFTS_MSG_DELAYRESPFLAGS, WFTS_MSG_DELAYRESPNFLAGS)) {
            tcLogger->error("Delayresp {} did not have expected flags", packet.packet_id);
            return WFTS_AWAIT_SYNC;
        }

        // Check for error
        if (packet.flags & WFTS_MSG_ERROR) {
            tcLogger->error("Delayresp {} had error flag set", packet.packet_id);
            return WFTS_AWAIT_SYNC;
        }
        closure->timestamps.t3 = packet.timestamp;
        return WFTS_COMPUTE_OFFSET;
    }

    // Computes offset with IEEE-1588 algorithm, send it to consumer. Shifts to AWAIT_SYNC
    uint32_t compute_offset_impl(FSMInterface* iface, void* rawClosure) {
        WF_DEBUGLOG(tcLogger, "SHIFTING TO COMPUTE_OFFSET");
        wfts_fsm_closure* closure = getClosure(rawClosure);
        
        auto offset = (closure->timestamps.t1 - closure->timestamps.t0 - closure->timestamps.t3 + closure->timestamps.t2)/2;
        closure->masterOffsetConsumer(offset);

        return WFTS_AWAIT_SYNC;
    }

    inline StateHandler fsm_dispatcher(uint32_t state, void*) {
        switch (state) {
            case WFTS_AWAIT_SYNC: return await_sync_impl;
            case WFTS_PROCESS_SYNC: return process_sync_impl;
            case WFTS_AWAIT_FOLLOWUP: return await_followup_impl;
            case WFTS_PROCESS_FOLLOWUP: return process_followup_impl;
            case WFTS_SEND_DELAYREQ: return send_delayreq_impl;
            case WFTS_PROCESS_DREQCTL: return process_dreqctl_impl;
            case WFTS_AWAIT_DELAYRESP: return await_delayresp_impl;
            case WFTS_PROCESS_DELAYRESP: return process_delayresp_impl;
            case WFTS_COMPUTE_OFFSET: return compute_offset_impl;
            default: return nullptr;
        }
    }
}

// NOTE: This code is particularly vulnerable to subtle sign-conversion logic errors.
// I'm 99% sure I did everything correctly, but just in case, this code should be
// audited if there are any otherwise unexplainable bugs in the future, especially regarding pose accuracy
// Especially the PHC offset calculation logic
namespace wf {
    using impl::tcLogger;
    // TODO: Maybe make WFTSClient a statusful object?
    WFTSClient::WFTSClient(std::unique_ptr<Socket> sock_, void (*masterOffsetConsumer_)(int64_t)) 
    : sock(std::move(sock_)) , servaddr{0}, servaddr_len{sizeof(servaddr)}
    , masterOffsetConsumer(masterOffsetConsumer_) {

        int enable = 1;
        // Enable broadcast support on the socket
        auto broadcast_res = sock->SetSockOpt(SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));
        if (!broadcast_res) {
            cleanup();
            throw wf_result_error(broadcast_res);
        }

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = WFTS_TIMEOUT_US;

        auto rcvtimeo_res = sock->SetSockOpt(SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        if (!rcvtimeo_res) {
            cleanup();
            throw wf_result_error(rcvtimeo_res);
        }
        auto sndtimeo_res = sock->SetSockOpt(SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        if (!sndtimeo_res) {
            cleanup();
            throw wf_result_error(sndtimeo_res);
        }

        tsopts |= (SOF_TIMESTAMPING_RAW_HARDWARE | SOF_TIMESTAMPING_SOFTWARE);

        auto fdopt = sock->getfd();

        if (fdopt) {
            // socket has a file descriptor, poll eth0 hardware timestamping capabilites
            const char* iface = "eth0"; // Wayfinder always uses the eth0 interface for network communications
            struct ifreq ifr;
            memset(&ifr, 0, sizeof(ifr));
            strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name)-1);

            struct ethtool_ts_info tsi;
            memset(&tsi, 0, sizeof(tsi));
            tsi.cmd = ETHTOOL_GET_TS_INFO;
            ifr.ifr_data = reinterpret_cast<char*>(&tsi);

            if (ioctl(fdopt.value(), SIOCETHTOOL, &ifr) < 0) {
                // TODO: Special error code for this?
                cleanup();
                throw wf_status_error(WFStatus::NETWORK_UNKNOWN, "Failed to retrieve eth0 NIC hardware information: {}", strerror(errno));
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

            // Open PHC file
            if (hwsupport) {
                if (tsi.phc_index < 0) {
                    cleanup();
                    throw wf_status_error(WFStatus::NETWORK_UNKNOWN, "NIC Reported hardware timestamping capabilities but does not have a valid PHC");
                }
                auto pathstr = std::format("/dev/ptp{}",tsi.phc_index);
                phcfd = open(pathstr.c_str(),O_RDONLY);
                if (phcfd < 0) {
                    cleanup();
                    throw wf_status_error(WFStatus::BAD_ACQUIRE,"Failed to acquire PHC: {}",strerror(errno));
                }
                phc_caps = new ptp_clock_caps;
                if (ioctl(phcfd, PTP_CLOCK_GETCAPS, phc_caps) < 0) {
                    cleanup();
                    throw wf_status_error(WFStatus::NETWORK_UNKNOWN,"Failed to query PHC capabilities: {}",strerror(errno));
                }
            } else {
                phc_caps = nullptr;
                phcfd = -1;
            }

        } else {
            // We are using a mock socket, default to software timestamping
            phc_caps = nullptr;
            phcfd = -1;
            tsopts |= (SOF_TIMESTAMPING_TX_SOFTWARE | SOF_TIMESTAMPING_RX_SOFTWARE);
        }

        // Apply tsopts to socket
        auto tsres = sock->SetSockOpt(SOL_SOCKET, SO_TIMESTAMPING, &tsopts, sizeof(tsopts));
        if (!tsres) {
            cleanup();
            throw wf_result_error(tsres);
        }

        fsmClosure = new impl::wfts_fsm_closure{
            .client = this,
            .getOffset = &WFTSClient::getOffset,
            .masterOffsetConsumer = masterOffsetConsumer,
            .servaddr = servaddr,
            .servaddr_len = servaddr_len,
            .sock = sock,
            .tsopts = tsopts,
            .timestamps = {0,0,0,0},
            .lastPacketID = 0,
            .offset = 0
        };

        stateMachine = std::make_unique<FiniteStateMachine>(impl::fsm_dispatcher, fsmClosure);
    }

    WFTSClient::~WFTSClient() {
        stop();
        cleanup();
    }


    void WFTSClient::cleanup() {
        if (phcfd >= 0) { close(phcfd); }
        if (phc_caps) { delete static_cast<ptp_clock_caps*>(phc_caps); }
        if (fsmClosure) { delete static_cast<impl::wfts_fsm_closure*>(fsmClosure); }
    }

    void WFTSClient::start() {
        if (worker.joinable()) return;
        worker = std::jthread([this](std::stop_token st){
            while (!st.stop_requested()) {
                this->stateMachine->step();
            }
        });
    }

    void WFTSClient::stop() {
        if (worker.joinable()) {
            worker.request_stop();
            worker.join();  // blocks until the worker finishes
            worker = std::jthread{};
        }
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
}