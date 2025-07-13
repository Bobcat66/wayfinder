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
    VisionWorkerManager::VisionWorkerManager(NetworkTablesManager& ntManager_, HardwareManager& hardwareManager_,ApriltagConfiguration& atagConfig_)
    : ntManager(ntManager_), hardwareManager(hardwareManager_), atagConfig(atagConfig_) {}
    VisionWorker& VisionWorkerManager::buildVisionWorker(const VisionWorkerConfig& config) {
        switch (config.pipelineType) {
            case PipelineType::Apriltag:
                {
                    if (!hardwareManager.cameraRegistered(config.devpath)) {
                        throw std::runtime_error(std::format("Camera {} not found",config.devpath));
                    }
                    if (!std::holds_alternative<ApriltagPipelineConfiguration>(config.pipelineConfig)) {
                        throw std::runtime_error(std::format("Pipeline {} is declared as Apriltag Pipeline yet specifies an incompatible configuration!",config.name));
                    }
                    std::vector<std::unique_ptr<CVProcessNode<cv::Mat>>> nodes;
                    FrameProvider& frameProvider = hardwareManager.getFrameProvider(config.devpath,std::format("{}_frameprovider",config.name));
                    StreamFormat hardwareFormat = hardwareManager.getStreamFormat(config.devpath);
                    if (config.inputFormat.frameFormat == hardwareFormat.frameFormat) {
                        nodes.emplace_back(std::move(std::make_unique<IdentityNode<cv::Mat>>()));
                    } else {
                        if (
                            config.inputFormat.frameFormat.rows != hardwareFormat.frameFormat.rows
                            || config.inputFormat.frameFormat.cols != hardwareFormat.frameFormat.cols
                        ) {
                            logger->warn("Resolution for pipeline {} differs from native resolution for camera {}. This could cause issues with calibration",config.name,config.devpath);
                            nodes.push_back(std::move(std::make_unique<ResizeNode<cv::Mat>>(
                                config.inputFormat.frameFormat.cols,
                                config.inputFormat.frameFormat.rows
                            )));
                        }
                        if (config.inputFormat.frameFormat.encoding != hardwareFormat.frameFormat.encoding) {
                            nodes.push_back(std::move(std::make_unique<ColorConvertNode<cv::Mat>>(
                                config.inputFormat.frameFormat.encoding
                            )));
                        }
                    }
                    CVProcessPipe preprocesser(hardwareFormat.frameFormat,std::move(nodes));
                    ApriltagPipeline pipeline(
                        std::get<ApriltagPipelineConfiguration>(config.pipelineConfig),
                        hardwareManager.get);
                }
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