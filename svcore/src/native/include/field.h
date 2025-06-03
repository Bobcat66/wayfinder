/*
 * Copyright (c) 2025 Jesse Kane
 * You may use, distribute and modify this code under the terms of the BSD 3-Clause License.
 * For more information, see the LICENSE file in the root directory of this project.
 */

#pragma once

#include <gtsam/geometry/Pose3.h>

namespace svcore {

typedef struct {
    int id;
    gtsam::Pose3 pose;
} Fiducial;

}