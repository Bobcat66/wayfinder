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

#include "wfcore/network/Socket.h"
#include <unordered_map>
#include <wpi/timestamp.h>
#include <memory>
#include <functional>

namespace wf {
   
    enum class MockNode {
        Client,
        Server,
        None
    };

    struct wfts_msg_record {
        wfts_msg_record(bool received_, const void* payload_, const void* controls_);
        bool operator==(const wfts_msg_record& other) const;
        std::string string() const;
        std::string controls_str(size_t rowlen = 0) const;
        const bool received;
        char payload[13];
        char controls[512];
    };

    using MsgConsumer = std::function<void(wfts_msg_record&&)>;
    
    struct MockTimeServer;
    // This is a mock unit testing harness which uses a pair of bound UNIX sockets and a clock to act as a fake server
    class MockTimeSocket : public Socket {
    public:
        // optConsumer and msgConsumer are callbacks for logging socket options and messages sent and received by the socket
        MockTimeSocket(MsgConsumer msgConsumer_, int32_t offset_us, bool sync_hastime = false);
        ~MockTimeSocket() noexcept;
        WFStatusResult SendMsg(const struct msghdr* msg, int flags = 0) override;
        WFStatusResult RecvMsg(struct msghdr* msg, int flags = 0) override;
        WFStatusResult SetSockOpt(int level, int option, const void* optval, socklen_t optlen) override;
        WFStatusResult GetSockOpt(int level, int option, void* optval, socklen_t* optlen) override;
        std::optional<int> getfd() override { return std::nullopt; }
        int geterr() override { return sockerr; };
        MockTimeSocket(const MockTimeSocket&) = delete;
        MockTimeSocket& operator=(const MockTimeSocket&) = delete;
    private:
        int sockerr;
        // the file descriptor for the "local" socket
        int local_fd;
        // the file descriptor for the "remote" socket
        int remote_fd;
        // These are output values. While the MockTimeSocket is running, it "owns" these, and accessing them externally may result in UB.
        // When the MockTimeSocket is destroyed, it becomes safe to read from the vectors again
        MsgConsumer msgConsumer;
        std::unique_ptr<MockTimeServer> mockServer;
    };
}