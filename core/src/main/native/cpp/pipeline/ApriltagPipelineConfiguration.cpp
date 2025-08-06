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

#include "wfcore/pipeline/ApriltagPipelineConfiguration.h"
#include "jval/ApriltagPipelineConfig.jval.hpp"
#include "wfcore/configuration/WFDefaults.h"

namespace wf {
    const jval::JSONValidationFunctor* ApriltagPipelineConfiguration::getValidator_impl() {
        return jval::get_ApriltagPipelineConfig_validator();
    }
    WFResult<ApriltagPipelineConfiguration> ApriltagPipelineConfiguration::fromJSON_impl(const JSON& jobject) {
        auto valid = validate(jobject);
        if (!valid) return WFResult<ApriltagPipelineConfiguration>::propagateFail(valid);

        ApriltagDetectorConfig detConfig = WFDefaults::tagDetectorConfig();
        QuadThresholdParams qtps = WFDefaults::qtps();
        if (jobject.contains("detConfig")) {
            auto dtc_jobject = jobject["detConfig"];
            detConfig = {
                dtc_jobject["numThreads"].get<int>(),
                dtc_jobject["quadDecimate"].get<float>(),
                dtc_jobject["quadSigma"].get<float>(),
                dtc_jobject["refineEdges"].get<bool>(),
                dtc_jobject["decodeSharpening"].get<double>(),
                dtc_jobject["debug"].get<bool>()
            };
        }
        if (jobject.contains("detQTPs")) {
            auto qtp_jobject = jobject["detQTPs"];
            qtps = {
                qtp_jobject["minClusterPixels"].get<int>(),
                qtp_jobject["maxNumMaxima"].get<int>(),
                qtp_jobject["criticalAngleRads"].get<float>(),
                qtp_jobject["maxLineFitMSE"].get<float>(),
                qtp_jobject["minWhiteBlackDiff"].get<int>(),
                qtp_jobject["deglitch"].get<bool>()
            };
        }
        // TODO: Move these into WFDefaults???
        auto solvePnP = getJSONOpt<bool>(jobject,"solvePnP",false);
        auto detectorExcludes = getJSONOpt<std::vector<int>>(jobject,"detectorExcludes",{});
        auto solvePnPExcludes = getJSONOpt<std::vector<int>>(jobject,"solvePnPExcludes",{});
        auto solveTagRelative = getJSONOpt<bool>(jobject,"solveTagRelative",false);
        auto tagField = getJSONOpt<std::string>(jobject,"tagField",WFDefaults::tagField());
        auto tagFamily = getJSONOpt<std::string>(jobject,"tagFamily",WFDefaults::tagFamily());
        auto tagSize = getJSONOpt<double>(jobject,"tagSize",WFDefaults::tagSize());
        return WFResult<ApriltagPipelineConfiguration>::success(
            std::in_place,
            solvePnP,
            std::move(detConfig),
            std::move(qtps),
            std::move(tagField),
            std::move(tagFamily),
            tagSize,
            detectorExcludes,
            solvePnPExcludes,
            solveTagRelative
        );
    }
    WFResult<JSON> toJSON_impl(const ApriltagPipelineConfiguration& object) {
        try {
            JSON jobject = {
                {"detConfig", {
                    {"numThreads", object.detConfig.numThreads},
                    {"quadDecimate", object.detConfig.quadDecimate},
                    {"quadSigma", object.detConfig.quadSigma},
                    {"refineEdges", object.detConfig.refineEdges},
                    {"decodeSharpening", object.detConfig.decodeSharpening},
                    {"debug", object.detConfig.debug}
                }},
                {"detQTPs", {
                    {"minClusterPixels", object.detQTPs.minClusterPixels},
                    {"maxNumMaxima", object.detQTPs.maxNumMaxima},
                    {"criticalAngleRads", object.detQTPs.criticalAngleRads},
                    {"maxLineFitMSE", object.detQTPs.maxLineFitMSE},
                    {"minWhiteBlackDiff", object.detQTPs.minWhiteBlackDiff},
                    {"deglitch", object.detQTPs.deglitch}
                }},
                {"tagField", object.apriltagField},
                {"tagFamily", object.apriltagFamily},
                {"tagSize", object.apriltagSize},
                {"detectorExcludes", object.detectorExcludes},
                {"solvePnPExcludes", object.SolvePNPExcludes},
                {"solveTagRelative", object.solveTagRelative}
            };
            return WFResult<JSON>::success(std::move(jobject));
        } catch (const JSON::exception& e) {
            return WFResult<JSON>::failure(WFStatus::JSON_UNKNOWN,e.what());
        }
    }
}