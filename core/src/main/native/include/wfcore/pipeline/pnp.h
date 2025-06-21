/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of Wayfinder.
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

#include "wfcore/configuration/configurations.h"
#include "wfcore/fiducial/ApriltagField.h"
#include "wfcore/fiducial/ApriltagDetection.h"

#include <opencv2/core.hpp>
#include <gtsam/geometry/Pose3.h>

#include <optional>
#include <unordered_set>

namespace wf {

    struct ApriltagRelativePoseObservation {
        int id;
        std::vector<cv::Point2d> corners;
        double decisionMargin;
        double hammingDistance;
        cv::Mat rvec0;
        cv::Mat tvec0;
        double error0;
        cv::Mat rvec1;
        cv::Mat tvec1;
        double error1;
        ApriltagRelativePoseObservation(
            int id_,
            const std::vector<cv::Point2d> corners_,
            double decisionMargin_, double hammingDistance_,
            cv::Mat rvec0_, cv::Mat tvec0_, double error0_,
            cv::Mat rvec1_, cv::Mat tvec1_, double error1_
        ) : id(id_), 
            corners(std::move(corners_)), 
            decisionMargin(decisionMargin_), hammingDistance(hammingDistance_),
            rvec0(std::move(rvec0_)), tvec0(std::move(tvec0_)), error0(error0_), 
            rvec1(std::move(rvec1_)), tvec1(std::move(tvec1_)), error1(error1_) {}
    };
    // Observation of a single Apriltag's pose relative to the camera

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
    };

    std::optional<ApriltagFieldPoseObservation> solvePNPApriltag(
        const std::vector<ApriltagDetection>& observations,
        const ApriltagField& fieldLayout,
        const CameraIntrinsics& cameraIntrinsics,
        const std::unordered_set<int>& ignoreList
    ) noexcept;

    std::optional<ApriltagRelativePoseObservation> solvePNPApriltagRelative(
        const ApriltagDetection& observation,
        const ApriltagField& fieldLayout,
        const CameraIntrinsics& cameraIntrinsics
    ) noexcept;

}