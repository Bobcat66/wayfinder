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

#include "wfcore/network/WFTSManager.h"
#include "wfcore/network/WFTSClientHandler.h"
#include "wfcore/common/logging.h"
#include "wfcore/network/MasterTime.h"
namespace wf {
    WFTSManager::WFTSManager(const NetworkTablesManager& ntman)
    : wfts_listener_handle{0} {
        wfts_eventsub = ntman.getControlTable()->GetBooleanTopic("WFTS").Subscribe(false);
    }

    void WFTSManager::startListener() {
        if (wfts_listener_handle == 0) {
            auto inst = nt::NetworkTableInstance::GetDefault();
            wfts_listener_handle = inst.AddListener(
                wfts_eventsub,
                nt::EventFlags::kValueRemote,
                [](const nt::Event& event) {
                    auto value = event.GetValueEventData()->value;
                    if (!value.IsBoolean()) {
                        globalLogger()->warn("wayfinder/control/WFTS published a non-boolean type");
                        return;
                    }
                    auto wftsEnabled = value.GetBoolean();
                    if (wftsEnabled) {
                        WFTSClientHandler::getInstance().startClient();
                        loadMasterTimeSupplier(WFTSClientHandler::getInstance().getTimeSupplier());
                    } else {
                        unloadMasterTimeSupplier();
                        WFTSClientHandler::getInstance().stopClient();
                    }
                }
            );
        }
    }

    void WFTSManager::stopListener() {
        if (wfts_listener_handle != 0) {
            auto inst = nt::NetworkTableInstance::GetDefault();
            inst.RemoveListener(wfts_listener_handle);
            wfts_listener_handle = 0;
        }
        WFTSClientHandler::getInstance().stopClient();
    }
}