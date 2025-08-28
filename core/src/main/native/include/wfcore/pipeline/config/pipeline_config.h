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

#include <variant>
#include "wfcore/pipeline/config/ApriltagPipelineConfiguration.h"
#include "wfcore/pipeline/config/ObjectDetectionPipelineConfiguration.h"
#include "wfcore/pipeline/PipelineType.h"
#include "wfcore/utils/LambdaVisitor.h"


namespace wf {
    using PipelineConfigVariant = std::variant<
        std::monostate,
        ApriltagPipelineConfiguration,
        ObjectDetectionPipelineConfiguration
    >;

    constexpr inline PipelineType getConfigType(const PipelineConfigVariant& cfg) {
        return std::visit(LambdaVisitor{
            [](const std::monostate&) { return PipelineType::NullType; },
            [](const ApriltagPipelineConfiguration& apcfg) { return PipelineType::Apriltag; },
            [](const ObjectDetectionPipelineConfiguration& odpcfg) { return PipelineType::ObjDetect; }
        }, cfg);
    }

}