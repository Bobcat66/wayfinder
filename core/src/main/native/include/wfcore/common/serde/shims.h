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
#include "wips/pose3.wips.h"
#include <gtsam/geometry/Pose3.h>

namespace wf {
    wips_pose3_t pose3_wfcore2wips(const gtsam::Pose3& pose);
    gtsam::Pose3 pose3_wips2wfcore(const wips_pose3_t& pose);
    wips_blob_t* packPose3(const gtsam::Pose3& pose);
    gtsam::Pose3 unpackPose3(wips_blob_t* data);
}