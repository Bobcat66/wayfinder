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

#include "wfcore/network/WFTSClientHandler.h"
#include "wfcore/common/wfassert.h"
#include "wfcore/network/WFTSClient.h"
#include <ntcore.h>
#include <mutex>
#include <wpi/timestamp.h>
#include "wfcore/network/UDPIPSocket.h"

namespace wf {
    WFStatusResult WFTSClientHandler::startClient() {
        if (client) return WFStatusResult::success();
        try {
            std::unique_ptr<Socket> sock = std::make_unique<UDPIPSocket>(WFTS_CLIENT_PORT);
            client = std::make_unique<WFTSClient>(std::move(sock),&WFTSClientHandler::loadMasterOffset);
            return WFStatusResult::success();
        } catch (const wfexception& e) {
            return WFStatusResult::failure(e.status(),e.what());
        }
    }

    WFStatusResult WFTSClientHandler::stopClient() {
        if (client) {
            client->stop();
            client.reset();
        }
        return WFStatusResult::success();
    }

    TimeSupplier WFTSClientHandler::getTimeSupplier() {
        return WFTSClientHandler::getWFTSTime;
    }

    int64_t WFTSClientHandler::getWFTSTime() noexcept {
        return static_cast<int64_t>(wpi::Now()) - WFTSClientHandler::getInstance().masterOffset.load(std::memory_order_relaxed);
    }

    WFTSClientHandler::WFTSClientHandler() = default;

    WFTSClientHandler::~WFTSClientHandler() = default;

}