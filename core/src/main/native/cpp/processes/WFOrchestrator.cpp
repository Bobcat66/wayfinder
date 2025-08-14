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
#include <memory>
#include "wfcore/configuration/WFDefaults.h"
#include "wfcore/common/status.h"
#include "wfcore/common/wfexcept.h"
#include <filesystem>
#include <fstream>

namespace impl {
    using namespace wf;
    static WFStatusResult loadDefaults() {
        auto pathstropt = env::getVar("WF_DEFAULTS_PATH");
        if (!pathstropt) {
            auto enverr = static_cast<WFStatus>(env::getError());
            return WFStatusResult::failure(enverr);
        }
        auto path = std::filesystem::path(pathstropt.value());
        if (!std::filesystem::exists(path))
            return WFStatusResult::failure(
                WFStatus::FILE_NOT_FOUND,
                "{} not found",path.string()
            );

        std::ifstream file(path);

        if (!file.is_open())
            return WFStatusResult::failure(
                WFStatus::FILE_NOT_OPENED,
                "{} not opened",path.string()
            );

        JSON jobject;
        try {
            file >> jobject;
        } catch (const JSON::parse_error& e) {
            return WFStatusResult::failure(
                WFStatus::JSON_PARSE,
                e.what()
            );
        }

        auto loadres = WFDefaults::load(jobject);
        if (!loadres)
            return loadres;
        
        return WFStatusResult::success();
    }
}

namespace wf {
    WFOrchestrator::WFOrchestrator(WFSystemConfig config)
    : ntManager_(config.device_name,config.team,config.nt_server) 
    , resourceManager_(config.paths.resource_path,config.paths.local_path) 
    , inferenceEngineFactory_(resourceManager_) 
    , apriltagPipelineFactory_(resourceManager_) {
        resourceManager_.assignResourceSubdir("fields", config.paths.fields_rsubdir);
        workerManager_ = std::move(std::make_unique<VisionWorkerManager>(ntManager_, hardwareManager_, inferenceEngineFactory_, apriltagPipelineFactory_));
    }

    void WFOrchestrator::periodic() noexcept {
        hardwareManager_.periodic();
    }

    WFOrchestrator WFOrchestrator::createFromEnv() {
        auto dres = impl::loadDefaults();
        if (!dres)
            throw wf_result_error(dres);

        auto scres = WFSystemConfig::getFromEnv();
        if (!scres)
            throw wf_result_error(scres);

        return WFOrchestrator(std::move(scres.value()));
        
    }
}