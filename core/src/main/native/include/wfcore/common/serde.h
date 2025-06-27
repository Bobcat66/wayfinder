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

#include "wips/wips_runtime.h"
#include "wfcore/pipeline/pnp.h"
#include "wfcore/pipeline/Pipeline.h"
#include <gtsam/geometry/Pose3.h>
#include <vector>

namespace wf {
    wips_bin_t* packPose3(const gtsam::Pose3& pose);
    gtsam::Pose3 unpackPose3(wips_bin_t* data);

    wips_bin_t* packApriltagRelativePoseObservation(const ApriltagRelativePoseObservation& poseObservation);
    ApriltagRelativePoseObservation unpackApriltagRelativePoseObservation(wips_bin_t* data);

    wips_bin_t* packApriltagFieldPoseObservation(const ApriltagFieldPoseObservation& poseObservation);
    ApriltagFieldPoseObservation unpackApriltagFieldPoseObservation(wips_bin_t* data);

    //std::vector<unsigned char> packPipelineResult(const PipelineResult& pipelineResult);
    //PipelineResult unpackPipelineResult(const std::vector<unsigned char>& data);
}