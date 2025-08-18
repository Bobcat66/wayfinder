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

    VisionWorkerManager::VisionWorkerManager(NetworkTablesManager& ntManager_, HardwareManager& hardwareManager_, InferenceEngineFactory& engineFactory, ApriltagPipelineFactory& apriltagPipelineFactory_)
    : ntManager(ntManager_), hardwareManager(hardwareManager_), engineFactory(engineFactory)
    , WFLoggedStatusfulObject("VisionWorkerManager",LogGroup::General), apriltagPipelineFactory(apriltagPipelineFactory_) {}

    // TODO: Refactor this with Status codes?
    WFResult<std::shared_ptr<VisionWorker>> VisionWorkerManager::buildVisionWorker(VisionWorkerConfig config) {
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
                            "Pipeline {} is declared as Apriltag Pipeline yet specifies an incompatible configuration!",config.name
                        );
                    
                    auto pipelineConfig = std::get<ApriltagPipelineConfiguration>(config.pipelineConfig);
                    auto intrinsics_res = hardwareManager.getIntrinsics(config.camera_nickname);
                    if ((!intrinsics_res) && (pipelineConfig.solvePnP))
                        return WFResult<std::shared_ptr<VisionWorker>>::failure(
                            WFStatus::APRILTAG_NO_INTRINSICS,
                            "Pipeline {} is configured to solve PnP, but no intrinsics were provided", config.name
                        );
                    CameraIntrinsics intrinsics = intrinsics_res
                        ? std::move(intrinsics_res.value())
                        : CameraIntrinsics{};

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
                    // Resolve input format and output format, if they are set to NULL
                    StreamFormat nullFormat;
                    if (config.inputFormat == nullFormat) 
                        config.inputFormat = hardwareFormat;

                    if (config.outputFormat == nullFormat)
                        config.outputFormat = hardwareFormat;

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
                    auto pipeline = apriltagPipelineFactory.createPipeline(
                        pipelineConfig,
                        intrinsics
                    );

                    // Build output consumer
                    auto outputConsumer = std::make_unique<ApriltagPipelineConsumer>(
                        config.name, config.camera_nickname,
                        intrinsics, 
                        config.inputFormat.frameFormat, 
                        config.raw_port, config.processed_port,
                        config.outputFormat,
                        pipelineConfig.apriltagSize,
                        ntManager.getDataPublisher(config.name)
                    );
                    outputConsumer->enableStreaming(config.stream);

                    // Build worker
                    auto worker = std::make_shared<VisionWorker>(
                        config.name,
                        frameProvider,
                        std::move(preprocesser),
                        std::move(pipeline.value()),
                        std::move(outputConsumer)
                    );
                    workers.insert({config.name,worker});
                    return worker;
                }
            case PipelineType::ObjDetect:
                return WFResult<std::shared_ptr<VisionWorker>>::failure(
                    WFStatus::NOT_IMPLEMENTED,
                    "Object Detection not implemented"
                ); // TODO: remove this once implemented
            default:
                return WFResult<std::shared_ptr<VisionWorker>>::failure(
                    WFStatus::UNKNOWN,
                    "How did you do this like actually. It should literally be impossible to get this error"
                );
        }
    }

    bool VisionWorkerManager::workerExists(const std::string& name) const {
        WF_DEBUGLOG(this->logger(),"Checking if worker {} exists",name);
        auto it = workers.find(name);
        return (it != workers.end());
    }

    WFResult<std::shared_ptr<VisionWorker>> VisionWorkerManager::getWorker(const std::string& name) {
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
        it->second->start();
    }

    void VisionWorkerManager::stopWorker(const std::string& name) {
        this->logger()->info("Stopping worker {}",name);
        auto it = workers.find(name);
        if (it == workers.end()) {
            this->logger()->warn("Worker {} not found",name);
            return;
        }
        if (!(it->second->isRunning())) {
            WF_DEBUGLOG(this->logger(),"Worker {} already stopped",name);
            return;
        }
        it->second->stop();
    }

    void VisionWorkerManager::destroyWorker(const std::string& name) {
        this->logger()->info("Destroying worker {}",name);
        auto it = workers.find(name);
        if (it == workers.end()) {
            this->logger()->warn("Worker {} not found",name);
            return;
        }
        WF_DEBUGLOG(this->logger(),"Stopping worker {}",it->first);
        it->second->stop();
        workers.erase(it);
    }

    void VisionWorkerManager::startAllWorkers() {
        this->logger()->info("Starting all workers");
        for (auto& [name,worker] : workers) {
            this->logger()->info("Starting worker {}",name);
            if (worker->isRunning()) { 
                WF_DEBUGLOG(this->logger(),"Worker {} already started",name);
                continue;
            }
            worker->start();
        }
    }

    void VisionWorkerManager::stopAllWorkers() {
        this->logger()->info("Stopping all workers");
        for (auto& [name,worker] : workers) {
            this->logger()->info("Stopping worker {}",name);
            if (!worker->isRunning()) {
                WF_DEBUGLOG(this->logger(),"Worker {} already stopped",name);
                continue;
            }
            worker->stop();
        }
    }

    void VisionWorkerManager::destroyAllWorkers() {
        this->logger()->info("Destroying all workers");
        for (auto it = workers.begin(); it != workers.end(); ) {
            this->logger()->info("Destroying worker {}",it->first);
            WF_DEBUGLOG(this->logger(),"Stopping worker {}",it->first);
            it->second->stop();
            it = workers.erase(it);
        }
    }
}