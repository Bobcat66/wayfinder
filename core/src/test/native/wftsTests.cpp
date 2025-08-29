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
#include "wfcore/network/MockTimeSocket.h"
#include <gtest/gtest.h>
#include "wfcore/common/logging.h"
#include <memory>
#include <thread>
#include <chrono>

void moffsetConsumer(int64_t offset) {
    wf::globalLogger()->info("Offset calculated as {} us", offset);
}

TEST(wftsTests,mocktest) {
    std::unique_ptr<wf::MockTimeSocket> sock = std::make_unique<wf::MockTimeSocket>([](wf::wfts_msg_record&& msgrec){
        wf::globalLogger()->info(msgrec.string());
    },1000);
    wf::WFTSClient client(std::move(sock),moffsetConsumer);
    client.start();
    std::this_thread::sleep_for(std::chrono::seconds(3));
    client.stop();
}