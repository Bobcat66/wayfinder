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

#include "wfcore/network/MockTimeSocket.h"
#include "wfcore/common/wfexcept.h"
#include <sys/socket.h>
#include <stdlib.h>
#include "wfcore/common/wfexcept.h"
#include <format>
#include <sstream>
#include <iostream>
#include "wfcore/common/logging.h"
#include <iomanip>
#include <thread>
#include <chrono>
#include "wfcore/utils/FiniteStateMachine.h"
#include "wfdetail/wfts/wfts_defs.h"
#include "wips/timesync_packet.wips.h"
#include <linux/net_tstamp.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#define MKSRV_SEND_SYNC 0           // Send SYNC message
#define MKSRV_PROCESS_SYNCCTL 1     // Process SYNC message ctl
#define MKSRV_SEND_FOLLOWUP 2       // Send followup
#define MKSRV_SERVE_DELAYREQS 3     // Serve delayreqs (first come first serve until next sync message is sent)
#define MKSRV_SEND_SIGERR 4         // Send an error signal to client

// The serve_delayreqs function itself uses an FSM
#define MKSRV_DRQSRV_AWAIT 0    // Await DELAYREQ
#define MKSRV_DRQSRV_RESPOND 1  // Respond with DELAYRESP
#define MKSRV_DRQSRV_SSIGERR 2  // Send error signal to client


#define MKSRV_SYNCFREQ_US 20000

namespace impl {
    using namespace wf;
    // returns the offset of the system clock relative to wpi::Now()
    // defined as sys(t) - wpi(t)
    static int64_t wpi_sys_offset() {
        auto t0 = std::chrono::system_clock::now();
        auto t1 = static_cast<int64_t>(wpi::Now());
        return std::chrono::duration_cast<std::chrono::microseconds>(
            t0.time_since_epoch()
        ).count() - t1;
    }
    // Returns a buffer as a string of hex codepoints
    std::string dumpBuf(const void* buf, size_t len, size_t rowlen = 0) {
        std::ostringstream oss;
        const char* charbuf = reinterpret_cast<const char*>(buf);
        oss << std::hex << std::setfill('0');
        for (size_t i = 0; i < len; ++i) {
            oss << std::setw(2) << static_cast<int>(static_cast<unsigned char>(charbuf[i]));
            // TS is scuffed but whatever
            if (rowlen) {
                if (i != (len - 1)) oss << (i % rowlen) ? " " : "\n";
            } else {
                if (i != (len - 1)) oss << " ";
            }
        }
        return oss.str();
    }
    StateHandler dreqsrv_fsm_dispatch(uint32_t,void*);
    StateHandler dispatch(uint32_t,void*);
}

namespace wf {

    // This is a simplified interface, a real server would have dedicated closure types
    struct MockTimeServer {
    public:
        // The socket FD is managed externally
        MockTimeServer(int sockfd_, int64_t offset_us_, int64_t network_delay_, bool sync_hastime_ = false)
        : sockfd(sockfd_), offset_us(offset_us_), network_delay(network_delay_)
        , sync_hastime(sync_hastime_) {
            int tsopts = SOF_TIMESTAMPING_RX_SOFTWARE | SOF_TIMESTAMPING_TX_SOFTWARE;
            if (setsockopt(sockfd, SOL_SOCKET, SO_TIMESTAMPING, &tsopts, sizeof(tsopts)) < 0)
                throw wf_status_error(WFStatus::NETWORK_BAD_SOCKOPT, strerror(errno));
            int flags = fcntl(sockfd, F_GETFL, 0);
            if (flags < 0) throw wf_status_error(WFStatus::POSIX_ERROR, strerror(errno));
            if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) throw wf_status_error(WFStatus::POSIX_ERROR, strerror(errno));
            server_fsm = std::make_unique<FiniteStateMachine>(impl::dispatch,this);
            dreqsrv_fsm = std::make_unique<FiniteStateMachine>(impl::dreqsrv_fsm_dispatch,this);
            worker = std::jthread([this](std::stop_token st){
                while (!st.stop_requested()) {
                    this->server_fsm->run();
                }
            });
        }
        uint64_t time_since_sync() {
            auto t = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<std::chrono::microseconds>(
                t - synctime
            ).count();
        }
        ~MockTimeServer() {
            if (worker.joinable()) {
                worker.request_stop();
                worker.join();
            }
        }
        uint32_t syncPacketID = 0;
        uint32_t lastPacketID = 0;
        const int64_t offset_us;
        const int64_t network_delay;
        const int sockfd;
        const bool sync_hastime;
        char payloadBuf[13];
        char controlBuf[512];
        // timestamp cache for passing timestamps between state handlers
        int64_t tscache = 0;
        // should only be called from the FSM
        void timestamp_sync() {
            synctime = std::chrono::steady_clock::now();
        }
        std::unique_ptr<FiniteStateMachine> dreqsrv_fsm;
    private: 
        std::chrono::time_point<std::chrono::steady_clock> synctime;
        std::jthread worker;
        std::unique_ptr<FiniteStateMachine> server_fsm;
    };

    wfts_msg_record::wfts_msg_record(bool received_, const void* payload_, const void* controls_)
    : received(received_) {
        memcpy(payload,payload_,WFTS_TSPACKET_SIZE);
        memcpy(controls,controls_,512);
    }
    bool wfts_msg_record::operator==(const wfts_msg_record& other) const {
        if (other.received != received) return false;
        if (memcmp(other.payload,payload,WFTS_TSPACKET_SIZE)) return false;
        if (memcmp(other.controls,controls,512)) return false;
        return true;
    }
    std::string wfts_msg_record::string() const {
        // decoding does not modify the blob buffer, so const_casting the payload is okay
        wips_blob_t blob = wips_blob_stackwrap(const_cast<char*>(payload),WFTS_TSPACKET_SIZE);
        wips_timesync_packet_t packet;
        memset(&packet, 0, sizeof(packet));
        auto res = wips_decode_timesync_packet(&packet, &blob);
        if (res.status_code != WIPS_STATUS_OK) throw wf_status_error(WFStatus::SERDE_UNKNOWN, "Error while decoding msgrecord buffer");
        // controls buffer is truncated due to size
        return std::format("{} id={}, time={}, flags={:#08b}",
            received ? "RECEIVED:" : "SENT:",
            packet.packet_id,
            packet.timestamp,
            packet.flags
        );
    }
    std::string wfts_msg_record::controls_str(size_t rowlen) const {
        return impl::dumpBuf(controls, 512, rowlen);
    }

    MockTimeSocket::MockTimeSocket(MsgConsumer msgConsumer_, int32_t network_delay_us_, int32_t offset_us_, bool sync_hastime) 
    : msgConsumer(msgConsumer_) {
        sockerr = 0;
        int sockfds[2];
        if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sockfds) < 0) {
            throw wf_status_error(WFStatus::BAD_ACQUIRE,"Failed to create UNIX domain sockets");
        }
        local_fd = sockfds[0];
        remote_fd = sockfds[1];
    }

    MockTimeSocket::~MockTimeSocket() noexcept {
        close(local_fd);
        close(remote_fd);
    }

    WFStatusResult MockTimeSocket::SendMsg(const struct msghdr* msg, int flags) {
        if (sendmsg(local_fd, msg, flags) < 0) {
            sockerr = errno;
            if (sockerr == EWOULDBLOCK || sockerr == EAGAIN) {
                // Socket would block, this is semi-nominal, return a lightweight WFStatusResult with no message
                return WFStatusResult::failure(WFStatus::NETWORK_WAITING);
            }
            return WFStatusResult::failure(WFStatus::NETWORK_BAD_SEND,strerror(errno));
        }
        sockerr = 0;
        wfts_msg_record msgrec(false,msg->msg_iov[0].iov_base,msg->msg_control);
        msgConsumer(std::move(msgrec));
        return WFStatusResult::success();
    }


    WFStatusResult MockTimeSocket::RecvMsg(struct msghdr* msg, int flags) {
        if (recvmsg(local_fd, msg, flags) < 0) {
            sockerr = errno;
            if (sockerr == EWOULDBLOCK || sockerr == EAGAIN) {
                // Socket would block, this is semi-nominal, return a lightweight WFStatusResult with no message
                return WFStatusResult::failure(WFStatus::NETWORK_WAITING);
            }
            return WFStatusResult::failure(WFStatus::NETWORK_BAD_RECV,strerror(errno));
        }
        sockerr = 0;
        wfts_msg_record msgrec(true,msg->msg_iov[0].iov_base,msg->msg_control);
        msgConsumer(std::move(msgrec));
        return WFStatusResult::success();
    }


    WFStatusResult MockTimeSocket::SetSockOpt(int level, int option, const void* optval, socklen_t optlen) {
        if (setsockopt(local_fd, level, option, optval, optlen) < 0) {
            sockerr = errno;
            return WFStatusResult::failure(WFStatus::NETWORK_BAD_SOCKOPT, strerror(sockerr));
        }
        globalLogger()->info("MockTimeSocket SetSockOpt: level={} option={} optlen={} optval={}",
            level,option,optlen,impl::dumpBuf(optval,optlen)
        );
        sockerr = 0;
        return WFStatusResult::success();
    }


    WFStatusResult MockTimeSocket::GetSockOpt(int level, int option, void* optval, socklen_t* optlen) {
        return WFStatusResult::failure(WFStatus::NOT_IMPLEMENTED);
    }
}

namespace impl {
    using namespace wf;

    static MockTimeServer* getClosure(void* closure) {
        return reinterpret_cast<MockTimeServer*>(closure);
    }

    uint32_t send_sync_impl(FSMInterface* iface, void* rawClosure) {
        auto closure = getClosure(rawClosure);
        closure->timestamp_sync();
        memset(closure->controlBuf, 0, 512);
        memset(closure->payloadBuf, 0, WFTS_TSPACKET_SIZE);
        closure->syncPacketID = closure->lastPacketID + 1;
        closure->lastPacketID = closure->syncPacketID;
        if (closure->sync_hastime) {
            // Send a one-stage sync message
            wips_timesync_packet_t packet;
            packet.flags = WFTS_MSG_SYNCFLAGS | WFTS_MSG_HASTIME;
            packet.packet_id = closure->syncPacketID;
            wips_blob_t blob = wips_blob_stackwrap(closure->payloadBuf,WFTS_TSPACKET_SIZE);
            packet.timestamp = wpi::Now(); // This won't be very accurate, but this is for unit testing anyways, so whatever
            auto res = wips_encode_nrb_timesync_packet(&blob, &packet);
            if (res.status_code != WIPS_STATUS_OK) {
                globalLogger()->error("MockServer failed to encode SYNC");
                return MKSRV_SEND_SYNC; // We don't need to send SIGERR because SIGERR just signals clients to expect a SYNC
            }
            if (send(closure->sockfd, closure->payloadBuf, sizeof(closure->payloadBuf), 0) < 0) {
                globalLogger()->error("MockServer failed to send SYNC: {}",strerror(errno));
                return MKSRV_SEND_SYNC;
            }
            return MKSRV_SERVE_DELAYREQS;
        } else {
            // Send a two-stage sync message

            // Prepare message
            wips_timesync_packet_t packet;
            packet.flags = WFTS_MSG_SYNCFLAGS;
            packet.packet_id = closure->syncPacketID;
            wips_blob_t blob = wips_blob_stackwrap(closure->payloadBuf,WFTS_TSPACKET_SIZE);
            auto res = wips_encode_nrb_timesync_packet(&blob, &packet);
            if (res.status_code != WIPS_STATUS_OK) {
                globalLogger()->error("MockServer failed to encode SYNC");
                return MKSRV_SEND_SYNC;
            }

            // Prepare message
            struct msghdr msg;
            struct iovec iov;

            iov.iov_base = closure->payloadBuf;
            iov.iov_len = sizeof(closure->payloadBuf);

            msg.msg_name = NULL;
            msg.msg_namelen = 0;
            msg.msg_iov = &iov;
            msg.msg_iovlen = 1;
            msg.msg_control = closure->controlBuf;
            msg.msg_controllen = sizeof(closure->controlBuf);

            if (sendmsg(closure->sockfd, &msg, 0) < 0) {
                globalLogger()->error("MockServer failed to send SYNC: {}",strerror(errno));
                return MKSRV_SEND_SYNC;
            }
            return MKSRV_PROCESS_SYNCCTL;
        }
    }

    uint32_t process_syncctl_impl(FSMInterface* iface, void* rawClosure) {
        auto closure = getClosure(rawClosure);
        memset(closure->controlBuf, 0, sizeof(closure->controlBuf));
        memset(closure->payloadBuf, 0, sizeof(closure->payloadBuf));

        struct msghdr msg;
        struct iovec iov = { nullptr, 0 }; // dummy iovec
        
        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = closure->controlBuf;
        msg.msg_controllen = sizeof(closure->controlBuf);

        if (recvmsg(closure->sockfd, &msg, MSG_ERRQUEUE) < 0) {
            globalLogger()->error("MockServer failed to process SYNCCTL: {}",strerror(errno));
            return MKSRV_SEND_SIGERR;
        }

        // Parse ancillary data
        struct cmsghdr* cmsg;
        // loop through all control messages in socket response
        for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
            if (cmsg->cmsg_type == SCM_TIMESTAMPING) {
                struct timespec* ts = reinterpret_cast<struct timespec*>(CMSG_DATA(cmsg));
                closure->tscache = (ts[0].tv_sec * 1e6) + (ts[0].tv_nsec/1000) - wpi_sys_offset();
                break;
            }
        }

        return MKSRV_SEND_FOLLOWUP;
    }

    uint32_t send_followup_impl(FSMInterface* iface, void* rawClosure) {
        auto closure = getClosure(rawClosure);
        wips_timesync_packet_t packet;
        packet.flags = WFTS_MSG_FOLLOWUPFLAGS;
        packet.packet_id = (closure->lastPacketID + 1);
        packet.timestamp = closure->tscache;
        wips_blob_t blob = wips_blob_stackwrap(closure->payloadBuf,WFTS_TSPACKET_SIZE);
        auto res = wips_encode_nrb_timesync_packet(&blob, &packet);
        if (sendto(closure->sockfd, closure->payloadBuf, sizeof(closure->payloadBuf), 0, nullptr, 0) < 0) {
            return MKSRV_SEND_SIGERR;
        }
        closure->lastPacketID++;
        return MKSRV_SERVE_DELAYREQS;
    }

    // This uses a simplified implementation of the DREQSRV FSM, as we only need to handle one client
    // This model is conceptually similar to how it would be implemented in a real server
    uint32_t serve_delayreqs_impl(FSMInterface* iface, void* rawClosure) {
        auto closure = getClosure(rawClosure);
        closure->dreqsrv_fsm.reset(MKSRV_DRQSRV_AWAIT);
        while (closure->time_since_sync() < MKSRV_SYNCFREQ_US) {
            // Calculate remaining time until next SYNC
            int64_t remaining_us = MKSRV_SYNCFREQ_US - closure->time_since_sync();
            int timeout_ms = static_cast<int>((remaining_us + 999) / 1000); // round up to milliseconds

            struct pollfd pfd;
            pfd.fd = closure->sockfd;
            pfd.events = POLLIN;
            pfd.revents = 0;

            int ret = poll(&pfd, 1, timeout_ms);
            if (ret > 0) {
                if (pfd.revents & POLLIN) {
                    // Socket has data — run FSM once to process a DELAYREQ
                    closure->dreqsrv_fsm->run();
                }
            } else if (ret == 0) {
                // Timeout — no data, loop again to check sync timer
                continue;
            } else {
                // poll error — log and continue
                globalLogger()->error("MockServer poll error: {}", strerror(errno));
                continue;
            }
        }
        return MKSRV_SEND_SYNC;
    }

    uint32_t send_sigerr_impl(FSMInterface* iface, void* rawClosure) {
        auto closure = getClosure(rawClosure);
        wips_timesync_packet_t packet;
        packet.flags = WFTS_MSG_BROADCAST | WFTS_MSG_ERROR | WFTS_MSG_LEADER;
        packet.packet_id = closure->syncPacketID;
        wips_blob_t blob = wips_blob_stackwrap(closure->payloadBuf,WFTS_TSPACKET_SIZE);
        if (sendto(closure->sockfd, closure->payloadBuf, sizeof(closure->payloadBuf), 0, nullptr, 0) < 0) {
            globalLogger()->error("MockServer failed to send SIGERR: {}",strerror(errno));
            return MKSRV_SEND_SIGERR; // attempt to send SIGERR until successful
        }
        return MKSRV_SEND_SYNC;
    }

    StateHandler dispatch(uint32_t state, void* dispatchContext) {
        (void)dispatchContext;
        switch (state) {
            case MKSRV_SEND_SYNC: return send_sync_impl;
            case MKSRV_PROCESS_SYNCCTL: return process_syncctl_impl;
            case MKSRV_SEND_FOLLOWUP: return send_followup_impl;
            case MKSRV_SERVE_DELAYREQS: return serve_delayreqs_impl;
            case MKSRV_SEND_SIGERR: return send_sigerr_impl;
            default: return nullptr;
        }
    }

    uint32_t drqsrv_await_impl(FSMInterface* iface, void* rawClosure) {
        auto closure = getClosure(rawClosure);
        struct msghdr msg;
        struct iovec iov;

        iov.iov_base = closure->payloadBuf;
        iov.iov_len = sizeof(closure->payloadBuf);

        msg.msg_name = nullptr;
        msg.msg_namelen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = closure->controlBuf;
        msg.msg_controllen = sizeof(closure->controlBuf);

        if (recvmsg(closure->sockfd, &msg, 0) < 0) {
            globalLogger()->error("MockServer failed to receive DELAYRE: {}", strerror(errno));
            return MKSRV_DRQSRV_AWAIT;
        }

        // Parse ancillary data
        struct cmsghdr* cmsg;
        // loop through all control messages in socket response
        for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
            if (cmsg->cmsg_type == SCM_TIMESTAMPING) {
                struct timespec* ts = reinterpret_cast<struct timespec*>(CMSG_DATA(cmsg));
                closure->tscache = (ts[0].tv_sec * 1e6) + (ts[0].tv_nsec/1000) - wpi_sys_offset();
                break;
            }
        }

        // Check packet
        wips_timesync_packet_t packet;
        memset(&packet, 0, sizeof(packet));
        wips_blob_t blob = wips_blob_stackwrap(closure->payloadBuf,sizeof(closure->payloadBuf));
        auto res = wips_decode_timesync_packet(&packet, &blob);
        if (res.status_code != WIPS_STATUS_OK) {
            globalLogger()->error("Failed to decode DELAYREQ");
            return MKSRV_DRQSRV_SSIGERR;
        }
        if (packet.packet_id != (closure->lastPacketID + 1)) {
            globalLogger()->error("DELAYREQ ID {} did not match expected {}", packet.packet_id, closure->lastPacketID + 1);
            return MKSRV_DRQSRV_SSIGERR;
        }
        if (!detail::flagcheck(packet.flags, WFTS_MSG_DELAYREQFLAGS, WFTS_MSG_DELAYREQNFLAGS)) {
            globalLogger()->error("DELAYREQ ID {} did not have expected flags", packet.packet_id);
            return MKSRV_DRQSRV_SSIGERR;
        }
        return MKSRV_DRQSRV_RESPOND;
    }

    uint32_t drqsrv_respond_impl(FSMInterface* iface, void* rawClosure) {
        auto closure = getClosure(rawClosure);
        wips_timesync_packet_t packet;
        packet.packet_id = (closure->lastPacketID + 2);
        packet.timestamp = closure->tscache;
        packet.flags = WFTS_MSG_DELAYRESPFLAGS;
        auto blob = wips_blob_stackwrap(closure->payloadBuf,sizeof(closure->payloadBuf));
        auto res = wips_encode_nrb_timesync_packet(&blob, &packet);
        if (res.status_code != WIPS_STATUS_OK) {
            globalLogger()->error("MockServer Failed to encode DELAYRESP");
            return MKSRV_DRQSRV_SSIGERR;
        }
        if (sendto(closure->sockfd, closure->payloadBuf, sizeof(closure->payloadBuf), 0, nullptr, 0) < 0) {
            globalLogger()->error("MockServer Failed to send DELAYRESP: {}", strerror(errno));
            return MKSRV_DRQSRV_SSIGERR;
        }
        return MKSRV_DRQSRV_AWAIT;
    }

    uint32_t drqsrv_ssigerr_impl(FSMInterface* iface, void* rawClosure) {
        auto closure = getClosure(rawClosure);
        wips_timesync_packet_t packet;
        packet.flags = WFTS_MSG_ERROR | WFTS_MSG_LEADER;
        packet.packet_id = closure->syncPacketID;
        wips_blob_t blob = wips_blob_stackwrap(closure->payloadBuf,WFTS_TSPACKET_SIZE);
        if (sendto(closure->sockfd, closure->payloadBuf, sizeof(closure->payloadBuf), 0, nullptr, 0) < 0) {
            globalLogger()->error("MockServer failed to send SIGERR: {}",strerror(errno));
            return MKSRV_DRQSRV_SSIGERR; // attempt to send SIGERR until successful
        }
        return MKSRV_DRQSRV_AWAIT;
    }

    StateHandler dreqsrv_fsm_dispatch(uint32_t state, void* dispatchContext) {
        (void)dispatchContext;
        switch (state) {
            case MKSRV_DRQSRV_AWAIT: return drqsrv_await_impl;
            case MKSRV_DRQSRV_RESPOND: return drqsrv_respond_impl;
            case MKSRV_DRQSRV_SSIGERR: return drqsrv_ssigerr_impl;
            default: return nullptr;
        }
    }



}