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

#include "wfcore/pipeline/ApriltagPipelineFactory.h"
#include "wfcore/common/wfexcept.h"

namespace wf {
    WFResult<std::unique_ptr<Pipeline>> ApriltagPipelineFactory::createPipeline(
        ApriltagPipelineConfiguration& config,
        CameraIntrinsics intrinsics
    ) {
        ApriltagFieldHandler handler(resourceManager);
        try {
            return std::make_unique<ApriltagPipeline>(
                config,
                std::move(intrinsics),
                handler
            );
        } catch (const wfexception& e) {
            return WFResult<std::unique_ptr<Pipeline>>::failure(
                e.status(),
                e.what()
            );
        }
    }
}