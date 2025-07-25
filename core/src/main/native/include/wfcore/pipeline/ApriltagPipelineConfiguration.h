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

#include "wfcore/fiducial/ApriltagDetector.h"
#include "wfcore/common/json_utils.h"

namespace wf {
    struct ApriltagPipelineConfiguration : JSONSerializable<ApriltagPipelineConfiguration> {
        ApriltagDetectorConfig detConfig;
        QuadThresholdParams detQTPs;
        std::unordered_set<int> detectorExcludes;
        std::unordered_set<int> SolvePNPExcludes; // Does not effect tag relative solvePNP
        bool solveTagRelative; // Whether or not to solve tag relative

        ApriltagPipelineConfiguration(
            ApriltagDetectorConfig detConfig_,
            QuadThresholdParams detQTPs_,
            std::unordered_set<int> detectorExcludes_,
            std::unordered_set<int> SolvePNPExcludes_,
            bool solveTagRelative_
        ) : detConfig(std::move(detConfig_))
        , detQTPs(std::move(detQTPs))
        , detectorExcludes(std::move(detectorExcludes_))
        , SolvePNPExcludes(std::move(SolvePNPExcludes_))
        , solveTagRelative(solveTagRelative_) {}

        ApriltagPipelineConfiguration(
            ApriltagDetectorConfig detConfig_,
            QuadThresholdParams detQTPs_,
            std::vector<int> detectorExcludes_,
            std::vector<int> SolvePNPExcludes_,
            bool solveTagRelative_
        ) : detConfig(std::move(detConfig_))
        , detQTPs(std::move(detQTPs))
        , detectorExcludes(detectorExcludes_.begin(),detectorExcludes_.end())
        , SolvePNPExcludes(SolvePNPExcludes_.begin(),SolvePNPExcludes_.end())
        , solveTagRelative(solveTagRelative_) {}
        
        static const JSONValidationFunctor* getValidator_impl();
        static WFResult<ApriltagPipelineConfiguration> fromJSON_impl(const JSON& jobject);
        static WFResult<JSON> toJSON_impl(const ApriltagPipelineConfiguration& object);
    };
}