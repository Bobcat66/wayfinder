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

namespace impl {
    using namespace wf;

    static const JSONValidationFunctor* getDetectorConfigValidator() {
        static JSONStructValidator validator(
            {
                {"numThreads", getPrimitiveValidator<int>()},
                {"quadDecimate", getPrimitiveValidator<float>()},
                {"quadSigma", getPrimitiveValidator<float>()},
                {"refineEdges", getPrimitiveValidator<bool>()},
                {"decodeSharpening", getPrimitiveValidator<double>()},
                {"debug", getPrimitiveValidator<bool>()}
            },
            {}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    static const JSONValidationFunctor* getDetectorQTPsValidator() {
        static JSONStructValidator validator(
            {
                {"minClusterPixels", getPrimitiveValidator<int>()},
                {"maxNumMaxima", getPrimitiveValidator<int>()},
                {"criticalAngleRads", getPrimitiveValidator<float>()},
                {"maxLineFitMSE", getPrimitiveValidator<float>()},
                {"minWhiteBlackDiff", getPrimitiveValidator<int>()},
                {"deglitch", getPrimitiveValidator<bool>()}
            },
            {}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    static const JSONValidationFunctor* getIntArrayValidator() {
        static JSONArrayValidator validator(getPrimitiveValidator<int>());
        return static_cast<JSONValidationFunctor*>(&validator);
    }
}

namespace wf {
    const JSONValidationFunctor* ApriltagPipelineConfiguration::getValidator_impl() {
        static JSONStructValidator validator(
            {
                {"detConfig", impl::getDetectorConfigValidator()},
                {"detQTPs", impl::getDetectorQTPsValidator()},
                {"detectorExcludes", impl::getIntArrayValidator()},
                {"solvePnPExcludes", impl::getIntArrayValidator()},
                {"solveTagRelative", getPrimitiveValidator<bool>()}
            },
            {}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }
    WFResult<ApriltagPipelineConfiguration> ApriltagPipelineConfiguration::fromJSON_impl(const JSON& jobject) {
        auto valid = (*getValidator())(jobject);
        if (!valid) return WFResult<ApriltagPipelineConfiguration>::propagateFail(valid);

        ApriltagDetectorConfig detConfig;
        QuadThresholdParams qtps;
        if (jobject.contains("detConfig")) {
            detConfig.numThreads = getJSONOpt<int>(jobject["detConfig"],"numThreads",detConfig.numThreads);
            detConfig.quadDecimate = getJSONOpt<float>(jobject["detConfig"],"quadDecimate",detConfig.quadDecimate);
            detConfig.quadSigma = getJSONOpt<float>(jobject["detConfig"],"quadSigma",detConfig.quadSigma);
            detConfig.refineEdges = getJSONOpt<bool>(jobject["detConfig"],"refineEdges",detConfig.refineEdges);
            detConfig.decodeSharpening = getJSONOpt<double>(jobject["detConfig"],"decodeSharpening",detConfig.decodeSharpening);
            detConfig.debug = getJSONOpt<bool>(jobject["detConfig"],"debug",detConfig.debug);
        }
        if (jobject.contains("detQTPs")) {
            qtps.minClusterPixels = getJSONOpt<int>(jobject["detQTPs"],"minClusterPixels",qtps.minClusterPixels);
            qtps.maxNumMaxima = getJSONOpt<int>(jobject["detQTPs"],"maxNumMaxima",qtps.maxNumMaxima);
            qtps.criticalAngleRads = getJSONOpt<float>(jobject["detQTPs"],"criticalAngleRads",qtps.criticalAngleRads);
            qtps.maxLineFitMSE = getJSONOpt<float>(jobject["detQTPs"],"maxLineFitMSE",qtps.maxLineFitMSE);
            qtps.minWhiteBlackDiff = getJSONOpt<int>(jobject["detQTPs"],"minWhiteBlackDiff",qtps.minWhiteBlackDiff);
            qtps.deglitch = getJSONOpt<bool>(jobject["detQTPs"],"deglitch",qtps.deglitch);
        }
        auto detectorExcludes = getJSONOpt<std::vector<int>>(jobject,"detectorExcludes",{});
        auto solvePnPExcludes = getJSONOpt<std::vector<int>>(jobject,"solvePnPExcludes",{});
        auto solveTagRelative = getJSONOpt<bool>(jobject,"solveTagRelative",true);
        return WFResult<ApriltagPipelineConfiguration>::success(
            std::in_place,
            std::move(detConfig),
            std::move(qtps),
            detectorExcludes,
            solvePnPExcludes,
            solveTagRelative
        );
    }
    WFResult<JSON> toJSON_impl(const ApriltagPipelineConfiguration& object) {
        return WFResult<JSON>::failure(WFStatus::UNKNOWN); // Placeholder
    }
}