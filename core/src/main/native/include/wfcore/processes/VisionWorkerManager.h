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
#include "wfcore/fiducial/ApriltagConfiguration.h"
#include "wfcore/common/status.h"
#include "wfcore/inference/InferenceEngineFactory.h"
#include <memory>

namespace wf {

    class VisionWorkerManager : public WFLoggedStatusfulObject {
    public:
        VisionWorkerManager(NetworkTablesManager& ntManager_, HardwareManager& hardwareManager_, InferenceEngineFactory& engineFactory_);
        WFResult<std::shared_ptr<VisionWorker>> buildVisionWorker(const VisionWorkerConfig& config);
        bool workerExists(const std::string& name) const;
        WFResult<std::shared_ptr<VisionWorker>> getWorker(const std::string& name);
        void startWorker(const std::string& name);
        void stopWorker(const std::string& name);
        void destroyWorker(const std::string& name);
        void startAllWorkers();
        void stopAllWorkers();
        void destroyAllWorkers();
        void periodic() noexcept;
    private:
        std::unordered_map<std::string,VisionWorker> workers;

        NetworkTablesManager& ntManager;
        HardwareManager& hardwareManager;
        InferenceEngineFactory& engineFactory;
    };
}