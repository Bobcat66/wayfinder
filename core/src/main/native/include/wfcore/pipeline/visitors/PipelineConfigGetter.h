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

#include "wfcore/pipeline/config/pipeline_config.h"
#include "wfcore/pipeline/pipelines/ApriltagPipeline.h"
#include "wfcore/pipeline/pipelines/ObjectDetectionPipeline.h"
#include "wfcore/common/status.h"

namespace wf {
    class PipelineConfigGetter : public PipelineVisitor {
    public:
        WFStatusResult operator()(ApriltagPipeline& pipeline) override;
        WFStatusResult operator()(ObjectDetectionPipeline& pipeline) override;
        template <typename T>
        WFResult<T> as() { 
            try {
                return std::get<T>(config);
            } catch (const std::bad_variant_access& e) {
                return WFResult<T>::failure(WFStatus::BAD_VARIANT,"Bad variant access");
            }
        }
        PipelineType getType() {
            return getConfigType(config);
        }
        PipelineConfigVariant get() { return config; }
    private:
        PipelineConfigVariant config;
    };
}