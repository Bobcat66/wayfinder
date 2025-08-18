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

#include "wfcore/processes/VisionWorkerManager.h"
#include "wfcore/hardware/HardwareManager.h"
#include "wfcore/network/NetworkTablesManager.h"
#include "wfcore/configuration/ResourceManager.h"
#include "wfcore/configuration/WFSystemConfig.h"
#include "wfcore/pipeline/ApriltagPipelineFactory.h"

namespace wf {
    class WFOrchestrator : public WFLoggedStatusfulObject {
    public:
        WFOrchestrator(WFSystemConfig config);
        void periodic() noexcept;
        // Stops all wayfinder processes except for the main thread
        void stopTheWorld();
        NetworkTablesManager& getNTManager() {
            return ntManager_;
        }
        HardwareManager& getHardwareManager() {
            return hardwareManager_;
        }
        ResourceManager& getResourceManager() {
            return resourceManager_;
        }
        VisionWorkerManager& getWorkerManager() {
            return workerManager_;
        }
        WFStatusResult configureHardware();
        WFStatusResult configureWorkers();
        static WFOrchestrator createFromEnv();
    private:
        NetworkTablesManager ntManager_;
        HardwareManager hardwareManager_;
        ResourceManager resourceManager_;
        VisionWorkerManager workerManager_;
        InferenceEngineFactory inferenceEngineFactory_;
        ApriltagPipelineFactory apriltagPipelineFactory_;
    };
}