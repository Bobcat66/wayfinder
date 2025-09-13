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

#include "wfcore/network/WFTSServer.h"
#include "wfdetail/wfts/wfts_defs.h"
#include "wips/timesync_packet.wips.h"
#include <linux/net_tstamp.h>
#include <poll.h>
#include "wfcore/common/wfexcept.h"
#include <netinet/in.h>
#include <arpa/inet.h>

#define WFTS_SEND_SYNC 0           // Send SYNC message
#define WFTS_PROCESS_SYNCCTL 1     // Process SYNC message ctl
#define WFTS_SEND_FOLLOWUP 2       // Send followup
#define WFTS_SERVE_DELAYREQS 3     // Serve delayreqs (first come first serve until next sync message is sent)
#define WFTS_SEND_SIGERR 4         // Send an error signal to client

#define CTL_BUF_SIZE 512

#define SERVER_FREQ_MS 40
#define SYNCCTL_TIMEOUT_MS 500
#define SRVDRQ_TIMEOUT_MS 5
#define WFTS_SERVER_PORT 35000

namespace impl {
    using namespace wf;

    struct message {
        struct msghdr hdr;
        struct iovec iov;
    };
    struct server_closure_t {
        const WFTSServer& server;
        std::unique_ptr<Socket>& sock;
        char controlBuf[CTL_BUF_SIZE];
        char payloadBuf[WFTS_TSPACKET_SIZE];
        struct {
            uint32_t id;
            int64_t ts;
        } cache;
        int& tsopts;
        const bool& sync_has_time;
        int64_t offset;
        struct sockaddr_storage broadcast_addr;
        // This address is modified while serving delayreqs
        struct sockaddr_storage client_addr;
        uint32_t last_broadcast_id;
        void timestamp_sync() {
            synctime = std::chrono::steady_clock::now();
        }
        uint64_t time_since_sync() {
            auto t = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                t - synctime
            ).count();
        }
        uint64_t time_until_sync() {
            int64_t rawval = SERVER_FREQ_MS - time_since_sync();
            if (rawval < 0) return 0;
            return rawval;
        }
        std::chrono::time_point<std::chrono::steady_clock> synctime;
        // Decodes payload and writes data into the packet;
        wips_result_t decode_payload(wips_timesync_packet_t* packet) {
            auto blob = wips_blob_stackwrap(payloadBuf,sizeof(payloadBuf));
            return wips_decode_timesync_packet(packet, &blob);
        }

        // Encodes the timestamp and packet ID in the cache into a packet, stores that packet in the payload buffer;
        wips_result_t encode_cache(uint8_t flags) {
            wips_timesync_packet_t packet;
            packet.packet_id = cache.id;
            packet.timestamp = cache.ts;
            packet.flags = flags;
            auto blob = wips_blob_stackwrap(payloadBuf,sizeof(payloadBuf));
            return wips_encode_nrb_timesync_packet(&blob, &packet);
        }

        // Reads error queue for the timestamp of the packet corresponding to cache.id, stores result in cache.ts
        WFStatusResult getTXTimestamp() {
             // Prepare message
            struct msghdr msg;
            struct iovec iov;

            iov.iov_base = payloadBuf;
            iov.iov_len = sizeof(payloadBuf);

            msg.msg_name = nullptr;
            msg.msg_namelen = 0;
            msg.msg_iov = &iov;
            msg.msg_iovlen = 1;
            msg.msg_control = controlBuf;
            msg.msg_controllen = sizeof(controlBuf);

            wips_timesync_packet_t packet;
            memset(&packet, 0, sizeof(packet));

            while (1) {
                auto pollres = sock->Poll(POLLERR,time_until_sync());
                if (!pollres) return WFStatusResult::propagateFail(pollres);

                if (!(pollres.value() & POLLERR)) continue; // POLLERR flag not set, continue

                auto recvRes = sock->RecvMsg(&msg, MSG_ERRQUEUE);
                if (!recvRes) return recvRes;

                auto decodeRes = decode_payload(&packet);
                if (decodeRes.status_code != WIPS_STATUS_OK)
                    return WFStatusResult::failure(WFStatus::SERDE_WIPS_UNKNOWN,"Failed to decode payload while parsing MSG_ERRQUEUE");
                if (packet.packet_id == cache.id) {
                    // Packet in error matches, break loop and process.
                    break;
                }
            }
            bool fetchedTS = false;
            // Parse ancillary data (rx timestamp)
            struct cmsghdr* cmsg;
            // loop through all control messages in socket response
            for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
                if ((cmsg->cmsg_level == SOL_SOCKET) && (cmsg->cmsg_type == SCM_TIMESTAMPING)) {
                    struct timespec* ts = reinterpret_cast<struct timespec*>(CMSG_DATA(cmsg));
                    // pmo stands for "pointer memory offset", and nothing else
                    int pmo = (tsopts & SOF_TIMESTAMPING_TX_HARDWARE) ? 2 : 0;
                    cache.ts = (ts[pmo].tv_sec * 1e6) + (ts[pmo].tv_nsec/1000) - offset;
                    fetchedTS = true;
                    break;
                }
            }

            if (!fetchedTS) {
                return WFStatusResult::failure(WFStatus::NETWORK_UNKNOWN,"Failed to retrieve TX timestamp");
            }
            return WFStatusResult::success();
        }

        void clearBuffers() {
            memset(controlBuf, 0, sizeof(controlBuf));
            memset(payloadBuf, 0, sizeof(payloadBuf));
        }

        message getmsg(struct sockaddr_storage* addr) {
            socklen_t addrlen = addr ? sizeof(*addr) : 0;

            message msg;

            msg.iov.iov_base = payloadBuf;
            msg.iov.iov_len = sizeof(payloadBuf);

            msg.hdr.msg_name = addr;
            msg.hdr.msg_namelen = addrlen;
            msg.hdr.msg_iov = &(msg.iov);
            msg.hdr.msg_iovlen = 1;
            msg.hdr.msg_control = controlBuf;
            msg.hdr.msg_controllen = sizeof(controlBuf);
            return msg;
        }

        WFStatusResult send(struct sockaddr_storage* addr, int flags = 0) {
            auto msg = getmsg(addr);
            return sock->SendMsg(&(msg.hdr),flags);
        }
    };

    static server_closure_t* getClosure(void* raw) {
        return static_cast<server_closure_t*>(raw);
    }

    // SYNC ID will be stored in cache.id after this operation
    static uint32_t send_sync_impl(FSMInterface* iface, void* rawClosure) {
        auto closure = getClosure(rawClosure);
        closure->clearBuffers();
        closure->offset = closure->server.getOffset();

        closure->cache.id++;
        auto res = closure->encode_cache(WFTS_MSG_SYNCFLAGS);
        if (res.status_code != WIPS_STATUS_OK) {
            globalLogger()->error("WFTS Server failed to encode SYNC");
            return WFTS_SEND_SYNC;
        }
        auto sendRes = closure->send(&(closure->broadcast_addr));
        if (!sendRes) {
            globalLogger()->error("WFTS Server failed to send SYNC: {}",sendRes.what());
            return WFTS_SEND_SYNC;
        }
        closure->last_broadcast_id = closure->cache.id;

        return WFTS_PROCESS_SYNCCTL;
    }

    // SYNC TX timestamp will be stored in cache.ts after this operation
    static uint32_t process_syncctl_impl(FSMInterface* iface, void* rawClosure) {
        auto closure = getClosure(rawClosure);
        // We store the previous packet so we can match the error message to the packet
        closure->clearBuffers();
        auto res = closure->getTXTimestamp();
        if (!res) {
            globalLogger()->error("Failed to retrieve SYNC timestamp: {}", res.what());
            return WFTS_SEND_SIGERR;
        }
        return WFTS_SEND_FOLLOWUP;
    }

    // FOLLOWUP packet id will be stored in cache.id after this operation
    static uint32_t send_followup_impl(FSMInterface* iface, void* rawClosure) {
        auto closure = getClosure(rawClosure);
        wips_timesync_packet_t packet;
        closure->cache.id++;
        auto res = closure->encode_cache(WFTS_MSG_FOLLOWUPFLAGS);
        auto sendRes = closure->sock->SendTo(
            closure->payloadBuf,
            sizeof(closure->payloadBuf),
            0,
            reinterpret_cast<sockaddr *>(&(closure->broadcast_addr)),
            sizeof(closure->broadcast_addr)
        );
        if (!sendRes) {
            globalLogger()->error("WFTS Server failed to send FOLLOWUP: {}",sendRes.what());
            return WFTS_SEND_SIGERR;
        }
        closure->last_broadcast_id = closure->cache.id;
        return WFTS_SERVE_DELAYREQS;
    }

    // TODO: Make the handler for fault-tolerant? This is just a test impl, so it doesn't really matter
    static uint32_t serve_delayreqs_impl(FSMInterface* iface, void* rawClosure) {
        auto closure = getClosure(rawClosure);
        while (closure->time_since_sync() < SERVER_FREQ_MS) {
            closure->cache.id = closure->last_broadcast_id;
            auto pollret = closure->sock->Poll(POLLIN, closure->time_until_sync());
            if (!pollret) {
                if (pollret.status() == WFStatus::NETWORK_WAITING) return WFTS_SEND_SYNC; // Timeout, go straight to SEND_SYNC
                globalLogger()->error("Error while serving DELAYREQs: {}",pollret.what());
                return WFTS_SEND_SIGERR;
            }

            auto msg = closure->getmsg(&(closure->client_addr));
            auto recvret = closure->sock->RecvMsg(&(msg.hdr));
            if (!recvret) {
                globalLogger()->error("Error while receiving DELAYREQ: {}",pollret.what());
                return WFTS_SEND_SIGERR;
            }

            wips_timesync_packet_t packet;
            auto decoderet = closure->decode_payload(&packet);
            if (decoderet.status_code != WIPS_STATUS_OK) {
                globalLogger()->error("Failed to decode DELAYREQ");

                // The packet ID and timestamp are junk values, we don't care about them
                closure->encode_cache(WFTS_MSG_ERROR);
                closure->send(&(closure->client_addr));

                continue;
            }

            if (packet.packet_id != (closure->last_broadcast_id + 1)) {
                globalLogger()->error(
                    "Packet {} did not have expected packet id {}",
                    packet.packet_id, closure->last_broadcast_id + 1
                );

                // The timestamp and cache ID are junk values, we don't care about them
                closure->encode_cache(WFTS_MSG_ERROR);
                closure->send(&(closure->client_addr));

                continue;
            }

            if (!detail::flagcheck(packet.flags, WFTS_MSG_DELAYREQFLAGS, WFTS_MSG_DELAYREQNFLAGS)) {
                globalLogger()->error(
                    "Packet {} did not have expected flags",
                    packet.packet_id
                );

                // The timestamp and cache ID are junk values, we don't care about them
                closure->encode_cache(WFTS_MSG_ERROR);
                closure->send(&(closure->client_addr));

                continue;
            }

            // Parse ancillary data (rx timestamp)
            bool fetchedTS = false;
            struct cmsghdr* cmsg;
            // loop through all control messages in socket response
            for (cmsg = CMSG_FIRSTHDR(&(msg.hdr)); cmsg != NULL; cmsg = CMSG_NXTHDR(&(msg.hdr), cmsg)) {
                if ((cmsg->cmsg_level == SOL_SOCKET) && (cmsg->cmsg_type == SCM_TIMESTAMPING)) {
                    struct timespec* ts = reinterpret_cast<struct timespec*>(CMSG_DATA(cmsg));
                    // pmo stands for "pointer memory offset", and nothing else
                    int pmo = (closure->tsopts & SOF_TIMESTAMPING_RX_HARDWARE) ? 2 : 0;
                    closure->cache.ts = (ts[pmo].tv_sec * 1e6) + (ts[pmo].tv_nsec/1000) - closure->offset;
                    fetchedTS = true;
                    break;
                }
            }

            if (!fetchedTS) {
                // Failed to fetch timestamp, send error to client
                // The timestamp and cache ID are junk values, we don't care about them
                closure->encode_cache(WFTS_MSG_ERROR);
                closure->send(&(closure->client_addr));
                continue;
            }
            

            // Respond
            closure->cache.id = closure->last_broadcast_id + 2;
            closure->encode_cache(WFTS_MSG_DELAYRESPFLAGS); // There isn't much we can do if this fails, but maybe log the error? Do this for other points of failure as well
            closure->send(&(closure->client_addr));
        }
        return WFTS_SEND_SYNC;
    }

    static StateHandler dispatch(uint32_t state, void* dispatchContext) {
        switch (state) {
            case WFTS_SEND_SYNC: return send_sync_impl;
            case WFTS_PROCESS_SYNCCTL: return process_syncctl_impl;
            case WFTS_SEND_FOLLOWUP: return send_followup_impl;
            case WFTS_SERVE_DELAYREQS: return serve_delayreqs_impl;
            default: return nullptr;
        }
    }
}

namespace wf {
    WFTSServer::WFTSServer(std::unique_ptr<Socket> sock_, int64_t (*timesource_)(void), const char* addr)
    : sock(std::move(sock_)), timesource(timesource_) {
        int enable = 1;
        int tsopts = SOF_TIMESTAMPING_SOFTWARE | SOF_TIMESTAMPING_RX_SOFTWARE | SOF_TIMESTAMPING_TX_SOFTWARE;
        auto tsopts_res = sock->SetSockOpt(SOL_SOCKET, SO_TIMESTAMPING, &tsopts, sizeof(tsopts));
        if (!tsopts_res) {
            throw wf_result_error(tsopts_res);
        }
        auto broadcast_res = sock->SetSockOpt(SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));
        if (!broadcast_res) {
            cleanup();
            throw wf_result_error(broadcast_res);
        }

        server_closure = new impl::server_closure_t {
            .server = *this,
            .sock = sock,
            .tsopts = tsopts,
            .sync_has_time = false
        };
        struct sockaddr_in* baddr_in = reinterpret_cast<struct sockaddr_in*>(&(impl::getClosure(server_closure)->broadcast_addr));
        memset(baddr_in,0,sizeof(struct sockaddr_in));
        baddr_in->sin_family = AF_INET;
        if (inet_pton(AF_INET, addr, &baddr_in->sin_addr.s_addr) != 1) {
            throw wf_status_error(WFStatus::POSIX_ERROR, strerror(errno));
        }
        baddr_in->sin_port = htons(WFTS_SERVER_PORT);

    }
    WFTSServer::~WFTSServer() {
        stop();
        cleanup();
        
    }
    void WFTSServer::cleanup() {
        if (server_closure) {
            delete impl::getClosure(server_closure);
        }
    }
    void WFTSServer::start() {
        
    }
    void WFTSServer::stop() {

    }
    int64_t WFTSServer::getOffset() const {
        return 0; // ts lwk scuffed
    }
}