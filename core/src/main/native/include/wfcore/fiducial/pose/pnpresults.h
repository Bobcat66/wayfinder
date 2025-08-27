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

#include <vector>
#include <opencv2/core.hpp>
#include <gtsam/geometry/Pose3.h>
#include <optional>

namespace wf {

    struct ApriltagRelativePoseObservation {
        int id; // Apriltag ID
        gtsam::Pose3 camPose0;
        double error0;
        gtsam::Pose3 camPose1;
        double error1;
        ApriltagRelativePoseObservation(
            int id_,
            gtsam::Pose3 camPose0_, double error0_,
            gtsam::Pose3 camPose1_, double error1_
        ) : id(id_), 
            camPose0(std::move(camPose0_)), error0(error0_), 
            camPose1(std::move(camPose1_)), error1(error1_) {}
    };

    struct ApriltagFieldPoseObservation {
        std::vector<int> tagsUsed;
        gtsam::Pose3 fieldPose0;
        double error0;
        std::optional<gtsam::Pose3> fieldPose1;
        std::optional<double> error1;

        ApriltagFieldPoseObservation(
            std::vector<int> tagsUsed_, 
            gtsam::Pose3 fieldPose0_, double error0_
        ) : tagsUsed(std::move(tagsUsed_)), 
            fieldPose0(std::move(fieldPose0_)), error0(error0_), 
            fieldPose1(std::nullopt), error1(std::nullopt) {}
        ApriltagFieldPoseObservation(
            std::vector<int> tagsUsed_, 
            gtsam::Pose3 fieldPose0_, double error0_,
            gtsam::Pose3 fieldPose1_, double error1_
        ) : tagsUsed(std::move(tagsUsed_)), 
            fieldPose0(std::move(fieldPose0_)), error0(error0_), 
            fieldPose1(std::make_optional(std::move(fieldPose1_))), error1(std::make_optional(error1_)) {}
        ApriltagFieldPoseObservation(
            std::vector<int> tagsUsed_, 
            gtsam::Pose3 fieldPose0_, double error0_,
            std::optional<gtsam::Pose3> fieldPose1_, std::optional<double> error1_
        ) : tagsUsed(std::move(tagsUsed_)), 
            fieldPose0(std::move(fieldPose0_)), error0(error0_), 
            fieldPose1(std::move(fieldPose1_)), error1(std::move(error1_)) {}
    };

}
