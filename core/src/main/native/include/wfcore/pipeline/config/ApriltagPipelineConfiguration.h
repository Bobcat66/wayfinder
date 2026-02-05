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
        bool solvePnP;
        ApriltagDetectorConfig detConfig;
        QuadThresholdParams detQTPs;
        std::string apriltagField;
        std::string apriltagFamily;
        double apriltagSize;
        std::unordered_set<int> detectorExcludes;
        std::unordered_set<int> SolvePNPExcludes; // Does not effect tag relative solvePNP
        bool solveTagRelative; // Whether or not to solve tag relative

        ApriltagPipelineConfiguration(
            bool solvePnP_,
            ApriltagDetectorConfig detConfig_,
            QuadThresholdParams detQTPs_,
            std::string apriltagField_,
            std::string apriltagFamily_,
            double apriltagSize_,
            std::unordered_set<int> detectorExcludes_,
            std::unordered_set<int> SolvePNPExcludes_,
            bool solveTagRelative_
        ) 
        : solvePnP(solvePnP_)
        , detConfig(std::move(detConfig_))
        , detQTPs(std::move(detQTPs))
        , apriltagField(std::move(apriltagField_))
        , apriltagFamily(std::move(apriltagFamily))
        , apriltagSize(apriltagSize_)
        , detectorExcludes(std::move(detectorExcludes_))
        , SolvePNPExcludes(std::move(SolvePNPExcludes_))
        , solveTagRelative(solveTagRelative_) {}

        ApriltagPipelineConfiguration(
            bool solvePnP_,
            ApriltagDetectorConfig detConfig_,
            QuadThresholdParams detQTPs_,
            std::string apriltagField_,
            std::string apriltagFamily_,
            double apriltagSize_,
            std::vector<int> detectorExcludes_,
            std::vector<int> SolvePNPExcludes_,
            bool solveTagRelative_
        )
        : solvePnP(solvePnP_)
        , detConfig(std::move(detConfig_))
        , detQTPs(std::move(detQTPs))
        , apriltagField(std::move(apriltagField_))
        , apriltagFamily(std::move(apriltagFamily))
        , apriltagSize(apriltagSize_)
        , detectorExcludes(detectorExcludes_.begin(),detectorExcludes_.end())
        , SolvePNPExcludes(SolvePNPExcludes_.begin(),SolvePNPExcludes_.end())
        , solveTagRelative(solveTagRelative_) {}
        
        static const jval::JSONValidationFunctor* getValidator_impl();
        static WFResult<ApriltagPipelineConfiguration> fromJSON_impl(const JSON& jobject);
        static WFResult<JSON> toJSON_impl(const ApriltagPipelineConfiguration& object);
    };
}