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

#include <vector>
#include <string>

#include "wfcore/processes/VisionWorkerConfig.h"
#include "wfcore/processes/VisionWorker.h"
#include "wfcore/hardware/HardwareManager.h"
#include "wfcore/network/NetworkTablesManager.h"

namespace wf {
    class VisionWorkerManager {
    public:
        VisionWorkerManager(NetworkTablesManager& ntManager_, HardwareManager& hardwareManager_);
        VisionWorker& buildVisionWorker(const VisionWorkerConfig& config);
        VisionWorker& getVisionWorker(const std::string& name);
        int startVisionWorker(const std::string& name);
        int stopVisionWorker(const std::string& name);
        int destroyVisionWorker(const std::string& name);
        int startAllWorkers();
        int stopAllWorkers();
        int destroyAllWorkers();
    private:
        std::vector<VisionWorker> workers;
        NetworkTablesManager& ntManager;
        HardwareManager& hardwareManager;
    };
}