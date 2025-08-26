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

#include <ntcore.h>
#include <networktables/BooleanTopic.h>
#include "wfcore/network/NetworkTablesManager.h"

namespace wf {
    class WFTSManager {
    public:
        WFTSManager(const NetworkTablesManager& ntman);
        ~WFTSManager() = default;
        // Causes the WFTSManager to start listening
        void startListener();
        // Causes the WFTSManager to stop listening
        void stopListener();
    private:
        nt::BooleanSubscriber wfts_eventsub;
        int wfts_listener_handle;
    };
}