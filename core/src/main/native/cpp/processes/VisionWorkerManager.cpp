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
#include "wfcore/common/logging.h"
#include "wfcore/pipeline/output/ApriltagPipelineConsumer.h"
#include "wfcore/common/wfexcept.h"
#include <format>
#include <stdexcept>

namespace wf {
    //static auto logger = LoggerManager::getInstance().getLogger("VisionWorkerManager",LogGroup::General);

    VisionWorkerManager::VisionWorkerManager(NetworkTablesManager& ntManager_, HardwareManager& hardwareManager_, InferenceEngineFactory& engineFactory)
    : ntManager(ntManager_), hardwareManager(hardwareManager_), engineFactory(engineFactory)
    , WFLoggedStatusfulObject("VisionWorkerManager",LogGroup::General) {}

    // TODO: Refactor this with Status codes?
    WFResult<std::shared_ptr<VisionWorker>> VisionWorkerManager::buildVisionWorker(const VisionWorkerConfig& config) {
        this->logger()->info("Building worker {}",config.name);
        switch (config.pipelineType) {
            case PipelineType::Apriltag:
                {
                    // Check to make sure the configuration passed is valid
                    if (!hardwareManager.cameraRegistered(config.camera_nickname))
                        return WFResult<std::shared_ptr<VisionWorker>>::failure(
                            WFStatus::HARDWARE_BAD_CAMERA, 
                            "Camera {} not registered", config.camera_nickname
                        );
                    if (!std::holds_alternative<ApriltagPipelineConfiguration>(config.pipelineConfig))
                        return WFResult<std::shared_ptr<VisionWorker>>::failure(
                            WFStatus::PIPELINE_BAD_CONFIG,
                            "Pipeline {} is declared as Apriltag Pipeline yet specifies an incompatible configuration!",config.camera_nickname
                        );
                    auto& intrinsics = hardwareManager.getIntrinsics(config.camera_nickname).value();
                    if (!intrinsics)
                        throw intrinsics_not_found(
                            std::format("Attempted to create apriltag PnP pipeline, but no camera intrinsics were specified for camera {} at the given resolution!",config.camera_nickname)
                        );

                    // Fetch frame provider from the hardware manager
                    auto frameProviderRes = hardwareManager.getFrameProvider(
                        config.camera_nickname,
                        std::format("{}_frameprovider",config.camera_nickname)
                    );
                    if (!frameProviderRes)
                        throw wf_result_error(frameProviderRes);
                    
                    auto frameProvider = std::move(frameProviderRes.value());
                        
                    auto hardwareFormatRes = frameProvider->getStreamFormat();
                    if (!hardwareFormatRes)
                        throw wf_result_error(hardwareFormatRes);
                    
                    auto hardwareFormat = std::move(hardwareFormatRes.value());
                    // Build preprocesser
                    std::vector<std::unique_ptr<CVProcessNode<cv::Mat>>> nodes;
                    if (config.inputFormat.frameFormat == hardwareFormat.frameFormat) {
                        nodes.emplace_back(std::move(std::make_unique<IdentityNode<cv::Mat>>()));
                    } else {
                        if (
                            config.inputFormat.frameFormat.height != hardwareFormat.frameFormat.height
                            || config.inputFormat.frameFormat.width != hardwareFormat.frameFormat.width
                        ) {
                            this->logger()->warn("Resolution for pipeline {} differs from native resolution for camera {}. This could cause issues with calibration",config.name,config.camera_nickname);
                            nodes.push_back(std::move(std::make_unique<ResizeNode<cv::Mat>>(
                                config.inputFormat.frameFormat.width,
                                config.inputFormat.frameFormat.height
                            )));
                        }
                        if (config.inputFormat.frameFormat.encoding != hardwareFormat.frameFormat.encoding) {
                            nodes.push_back(std::move(std::make_unique<ColorConvertNode<cv::Mat>>(
                                config.inputFormat.frameFormat.encoding
                            )));
                        }
                    }
                    CVProcessPipe preprocesser(hardwareFormat.frameFormat,std::move(nodes));
                    auto pipeline = std::make_unique<ApriltagPipeline>(
                        std::get<ApriltagPipelineConfiguration>(config.pipelineConfig),
                        intrinsics.value(),
                        atagConfig,
                        atagField
                    );

                    // Build output consumer
                    auto outputConsumer = std::make_unique<ApriltagPipelineConsumer>(
                        config.name, config.camera_nickname,
                        intrinsics.value(), 
                        config.inputFormat.frameFormat, 
                        config.raw_port, config.processed_port,
                        config.outputFormat,
                        atagConfig.tagSize,
                        ntManager.getDataPublisher(config.name)
                    );
                    outputConsumer->enableStreaming(config.stream);

                    // Build worker
                    workers.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(config.name),
                        std::forward_as_tuple(
                            config.name,
                            frameProvider,
                            std::move(preprocesser),
                            std::move(pipeline),
                            std::move(outputConsumer)
                        )
                    );
                }
            case PipelineType::ObjDetect:
                throw std::runtime_error("Object Detection not implemented"); // TODO: remove this once implemented
            default:
                throw std::runtime_error("How did you do this like actually. It should literally be impossible to get this error");
        }
    }

    bool VisionWorkerManager::workerExists(const std::string& name) const {
        WF_DEBUGLOG(this->logger(),"Checking if worker {} exists",name);
        auto it = workers.find(name);
        return (it != workers.end());
    }

    VisionWorker& VisionWorkerManager::getWorker(const std::string& name) {
        WF_DEBUGLOG(this->logger(),"Getting worker {}",name);
        auto it = workers.find(name);
        if (it == workers.end()) {
            throw vision_worker_not_found(std::format("Vision worker {} not found",name));
        }
        return it->second;
    }

    void VisionWorkerManager::startWorker(const std::string& name) {
        this->logger()->info("Starting worker {}",name);
        auto it = workers.find(name);
        if (it == workers.end()) {
            this->logger()->warn("Vision worker {} not found",name);
            return;
        }
        it->second.start();
    }

    void VisionWorkerManager::stopWorker(const std::string& name) {
        this->logger()->info("Stopping worker {}",name);
        auto it = workers.find(name);
        if (it == workers.end()) {
            this->logger()->warn("Worker {} not found",name);
            return;
        }
        if (!(it->second.isRunning())) {
            WF_DEBUGLOG(this->logger(),"Worker {} already stopped",name);
            return;
        }
        it->second.stop();
    }

    void VisionWorkerManager::destroyWorker(const std::string& name) {
        this->logger()->info("Destroying worker {}",name);
        auto it = workers.find(name);
        if (it == workers.end()) {
            this->logger()->warn("Worker {} not found",name);
            return;
        }
        WF_DEBUGLOG(this->logger(),"Stopping worker {}",it->first);
        it->second.stop();
        workers.erase(it);
    }

    void VisionWorkerManager::startAllWorkers() {
        this->logger()->info("Starting all workers");
        for (auto& [name,worker] : workers) {
            this->logger()->info("Starting worker {}",name);
            if (worker.isRunning()) { 
                WF_DEBUGLOG(this->logger(),"Worker {} already started",name);
                continue;
            }
            worker.start();
        }
    }

    void VisionWorkerManager::stopAllWorkers() {
        this->logger()->info("Stopping all workers");
        for (auto& [name,worker] : workers) {
            this->logger()->info("Stopping worker {}",name);
            if (!worker.isRunning()) {
                WF_DEBUGLOG(this->logger(),"Worker {} already stopped",name);
                continue;
            }
            worker.stop();
        }
    }

    void VisionWorkerManager::destroyAllWorkers() {
        this->logger()->info("Destroying all workers");
        for (auto it = workers.begin(); it != workers.end(); ) {
            this->logger()->info("Destroying worker {}",it->first);
            WF_DEBUGLOG(this->logger(),"Stopping worker {}",it->first);
            it->second.stop();
            it = workers.erase(it);
        }
    }
}