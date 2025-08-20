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

#include "wfcore/processes/WFOrchestrator.h"
#include "wfcore/common/wfexcept.h"
#include "wfcore/common/envutils.h"
#include "wfcore/configuration/WFDefaults.h"
#include "wfcore/hardware/CameraConfiguration.h"
#include <memory>
#include "wfcore/common/status.h"
#include "wfcore/common/wfexcept.h"
#include <filesystem>
#include <fstream>

namespace wf {
    WFOrchestrator::WFOrchestrator(WFSystemConfig config)
    : WFLoggedStatusfulObject("WFOrchestrator",LogGroup::General)
    , systemConfig_(config)
    , ntManager_(config.device_name,config.team,config.nt_server) 
    , resourceManager_(config.paths.resource_path,config.paths.local_path) 
    , inferenceEngineFactory_(resourceManager_) 
    , apriltagPipelineFactory_(resourceManager_) 
    , workerManager_(ntManager_, hardwareManager_, inferenceEngineFactory_, apriltagPipelineFactory_) {
        auto fields_res 
            = resourceManager_.assignResourceSubdir("fields", config.paths.fields_rsubdir);
        if (!fields_res) throw wf_result_error(fields_res);

        auto models_res 
            = resourceManager_.assignResourceSubdir("models", config.paths.models_rsubdir);
        if (!models_res) throw wf_result_error(models_res);

        auto pipelines_res 
            = resourceManager_.assignLocalSubdir("pipelines", config.paths.pipeline_lsubdir);
        if (!pipelines_res) throw wf_result_error(pipelines_res);

        auto hardware_res 
            = resourceManager_.assignLocalSubdir("hardware", config.paths.hardware_lsubdir);
        if (!hardware_res) throw wf_result_error(hardware_res);
    }

    void WFOrchestrator::periodic() noexcept {
        hardwareManager_.periodic();
    }

    WFOrchestrator WFOrchestrator::createFromEnv() {
        auto dres = loadDefaultsFromEnv();
        if (!dres) throw wf_result_error(dres);

        auto scres = WFSystemConfig::getFromEnv();
        if (!scres) throw wf_result_error(scres);

        return WFOrchestrator(std::move(scres.value()));
        
    }

    WFStatusResult WFOrchestrator::configureHardware() {
        auto enum_res = resourceManager_.enumerateLocalSubdir("hardware");
        if (!enum_res)
            return WFStatusResult::propagateFail(enum_res);
        auto config_files = std::move(enum_res.value());
        std::vector<CameraConfiguration> configs;
        for (const auto& config_file : config_files) {
            auto JSONLoadRes = resourceManager_.loadLocalJSON("hardware",config_file);
            if (!JSONLoadRes) {
                logger()->error(JSONLoadRes.what());
                continue;
            }
            auto decodeRes = CameraConfiguration::fromJSON(JSONLoadRes.value());
            if (!decodeRes){
                logger()->error(decodeRes.what());
                continue;
            }
            configs.push_back(std::move(decodeRes.value()));
        }
        for (const auto& config : configs) {
            auto res = hardwareManager_.registerCamera(config);
            if (!res) {
                logger()->warn("Failed to register camera {}",config.nickname);
            } else {
                logger()->info("Registered camera {}",config.nickname);
            }
        }
        return WFStatusResult::success();
    }

    WFStatusResult WFOrchestrator::configureWorkers() {
        auto enum_res = resourceManager_.enumerateLocalSubdir("pipelines");
        if (!enum_res)
            return WFStatusResult::propagateFail(enum_res);
        auto config_files = std::move(enum_res.value());
        std::vector<VisionWorkerConfig> configs;
        for (const auto& config_file : config_files) {
            auto JSONLoadRes = resourceManager_.loadLocalJSON("pipelines",config_file);
            if (!JSONLoadRes) {
                logger()->error(JSONLoadRes.what());
                continue;
            }
            auto decodeRes = VisionWorkerConfig::fromJSON(JSONLoadRes.value());
            if (!decodeRes){
                logger()->error(decodeRes.what());
                continue;
            }
            configs.push_back(std::move(decodeRes.value()));
        }
        for (auto& config : configs) {
            auto res = workerManager_.buildVisionWorker(config);
            if (!res) {
                logger()->error(res.what());
                continue;
            }
        }
        return WFStatusResult::success();
    }
}