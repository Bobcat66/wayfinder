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
#include <memory>
#include "wfcore/configuration/WFDefaults.h"

namespace wf {
    WFOrchestrator::WFOrchestrator(WFSystemConfig config)
    : ntManager_(config.device_name,config.team,config.nt_server) 
    , resourceManager_(config.paths.resource_path,config.paths.local_path) 
    , inferenceEngineFactory_(resourceManager_) {
        ApriltagConfiguration apriltagConfiguration(
            WFDefaults::getTagFamily(), 
            WFDefaults::getTagSize()
        );
        resourceManager_.assignResourceSubdir("aprilTagFields", config.paths.fields_rsubdir);
        auto tmp = resourceManager_.loadResourceJSON("aprilTagFields", "2025-reefscape-welded.json");
        if(!tmp) {
            throw std::runtime_error("Bad JSON");
        }
        auto value = tmp.value();
        aprilTagField_ = ApriltagField::fromJSON(value).value();

        workerManager_ = std::move(std::make_unique<VisionWorkerManager>(ntManager_, hardwareManager_, apriltagConfiguration, aprilTagField_, inferenceEngineFactory_));
    }

    void WFOrchestrator::periodic() noexcept {
        hardwareManager_.periodic();
    }
}