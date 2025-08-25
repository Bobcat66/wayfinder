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

namespace impl {
    using namespace wf;
    class WFTSClientHandler_impl {
    public:
        static WFTSClientHandler_impl& getInstance() {
            static WFTSClientHandler_impl inst;
            return inst;
        }
        WFStatusResult startClient() {
            if (client) return WFStatusResult::success();
            try {
                client = std::make_unique<WFTSClient>();
                return WFStatusResult::success();
            } catch (const wfexception& e) {
                return WFStatusResult::failure(e.status(),e.what());
            }
        }
        WFStatusResult stopClient() {
            if (client) {
                client->stop();
                client.reset();
            }
            return WFStatusResult::success();
        }
        TimeSupplier getTimeSupplier() {
            return WFTSClientHandler_impl::getWFTSTime;
        }
    private:
        WFTSClientHandler_impl();
        ~WFTSClientHandler_impl() = default;
        static int64_t getWFTSTime() {
            auto& inst = WFTSClientHandler_impl::getInstance();
            WF_FatalAssert(inst.client);
            return inst.client->getMasterOffset();
        }
        WFTSClientHandler_impl(const WFTSClientHandler_impl&) = delete;
        WFTSClientHandler_impl& operator=(const WFTSClientHandler_impl&) = delete;
        WFTSClientHandler_impl(WFTSClientHandler_impl&&) = delete;
        WFTSClientHandler_impl& operator=(WFTSClientHandler_impl&&) = delete;
        std::unique_ptr<WFTSClient> client;
    };
}

namespace wf {
    WFStatusResult WFTSClientHandler::startClient() {
        return impl::WFTSClientHandler_impl::getInstance().startClient();
    }
    WFStatusResult WFTSClientHandler::stopClient() {
        return impl::WFTSClientHandler_impl::getInstance().stopClient();
    }
    TimeSupplier WFTSClientHandler::getTimeSupplier() {
        return impl::WFTSClientHandler_impl::getInstance().getTimeSupplier();
    }
    
}