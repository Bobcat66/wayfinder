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

#include <memory>
#include "wfcore/common/status.h"
#include <shared_mutex>
#include <atomic>

namespace wf {
    class WFTSClient;
    using TimeSupplier = int64_t(*)(void);

    class WFTSClientHandler {
    public:
        static WFTSClientHandler& getInstance() {
            static WFTSClientHandler inst;
            return inst;
        }
        // startClient and stopClient are NOT safe to call, they are meant to only be called by the WFTSManager's listener
        WFStatusResult startClient();
        WFStatusResult stopClient();
        TimeSupplier getTimeSupplier();
    private:
        WFTSClientHandler();
        ~WFTSClientHandler();
        static int64_t getWFTSTime() noexcept;
        static void loadMasterOffset(int64_t masterOffset_) {
            WFTSClientHandler::getInstance().masterOffset.store(masterOffset_);
        }
        WFTSClientHandler(const WFTSClientHandler&) = delete;
        WFTSClientHandler& operator=(const WFTSClientHandler&) = delete;
        WFTSClientHandler(WFTSClientHandler&&) = delete;
        WFTSClientHandler& operator=(WFTSClientHandler&&) = delete;
        std::unique_ptr<WFTSClient> client;
        std::shared_mutex mtx;
        std::atomic_int64_t masterOffset{0};
    };
}