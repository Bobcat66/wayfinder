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

#include "wfcore/pipeline/Pipeline.h"
#include <vector>
#include <cstdint>
#include <gtsam/geometry/Pose3.h>
#include "wfcore/common/serde/serde_utils.h"
#include "wfcore/pipeline/pnp.h"

namespace wf {

    int packPose3(std::vector<byte>& out,const gtsam::Pose3& in) noexcept;
    gtsam::Pose3 unpackPose3(
        const std::vector<byte>& in,
        std::vector<byte>::const_iterator cit
    ) noexcept;

    int packPipelineResult(std::vector<byte>& out,const PipelineResult& in) noexcept;
    PipelineResult unpackPipelineResult(
        const std::vector<byte>& data,
        std::vector<byte>::const_iterator cit
    ) noexcept;

    int packApriltagRelativePoseObservation(
        std::vector<byte>& out,
        const ApriltagRelativePoseObservation& in
    ) noexcept;
    ApriltagRelativePoseObservation unpackApriltagRelativePoseObservation(
        const std::vector<byte>& data,
        std::vector<byte>::const_iterator cit
    ) noexcept;

    int packApriltagFieldPoseObservation(
        std::vector<byte>& out,
        const ApriltagFieldPoseObservation& in
    ) noexcept;
    ApriltagFieldPoseObservation unpackApriltagFieldPoseObservation(
        const std::vector<byte>& data,
        std::vector<byte>::const_iterator cit
    ) noexcept;

}