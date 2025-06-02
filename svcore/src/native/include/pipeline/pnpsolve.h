/*
 * Copyright (c) 2025 Jesse Kane
 * You may use, distribute and modify this code under the terms of the BSD 3-Clause License.
 * For more information, see the LICENSE file in the root directory of this project.
 */

#pragma once

#include "svtypes.h"

namespace svcore {

class IPPESquareSolver {
public:
    /**
     * @brief Solve the pose of a fiducial marker given its observations.
     * 
     * @param observations A vector of FiducialObservation containing the observed corners and their properties.
     * @return FiducialPoseObservation containing the estimated pose and associated errors.
     */
    FiducialPoseObservation solve(const FiducialObservation& observation);
};

}