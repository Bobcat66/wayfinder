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

// Commented out so the damn thing builds

#include "wfcore/processes/VisionWorkerManager.h"
#include "wfcore/video/processing.h"
#include "wfcore/hardware/HardwareManager.h"
#include "wfcore/network/NetworkTablesManager.h"
#include "wfcore/common/logging/LoggerManager.h"
#include <format>
#include <stdexcept>

namespace wf {
    static auto logger = LoggerManager::getInstance().getLogger("VisionWorkerManager",LogGroup::General);
    VisionWorkerManager::VisionWorkerManager(NetworkTablesManager& ntManager_, HardwareManager& hardwareManager_)
    : ntManager(ntManager_), hardwareManager(hardwareManager_) {}
    VisionWorker& VisionWorkerManager::buildVisionWorker(const VisionWorkerConfig& config) {
        switch (config.pipelineType) {
            case PipelineType::Apriltag:
                if (!hardwareManager.cameraRegistered(config.devpath)) {
                    logger->warn("Camera {} not found",config.devpath);
                }
                if (!config.pipeline)
            case PipelineType::ObjDetect:
                throw std::runtime_error("Object Detection not implemented"); // TODO: remove this once implemented
            case PipelineType::ApriltagDetect:
                throw std::runtime_error("Apriltag Detection not implemented");
            default:
                throw std::runtime_error("How did you do this like actually. It should literally be impossible to get this error");
        }
    }
    VisionWorker& getVisionWorker(const std::string& name);
    int startVisionWorker(const std::string& name);
    int stopVisionWorker(const std::string& name);
    int destroyVisionWorker(const std::string& name);
    int startAllWorkers();
    int stopAllWorkers();
    int destroyAllWorkers();
}