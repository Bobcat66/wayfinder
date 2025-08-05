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

#include "wfdetail/validation/tag_detector_validators.h"

namespace wf::detail {

    const JSONValidationFunctor* getDetectorConfigValidator() {
        static JSONStructValidator validator(
            {
                {"numThreads", getPrimitiveValidator<int>()},
                {"quadDecimate", getPrimitiveValidator<float>()},
                {"quadSigma", getPrimitiveValidator<float>()},
                {"refineEdges", getPrimitiveValidator<bool>()},
                {"decodeSharpening", getPrimitiveValidator<double>()},
                {"debug", getPrimitiveValidator<bool>()}
            },
            {},
            {}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    const JSONValidationFunctor* getDetectorQTPsValidator() {
        static JSONStructValidator validator(
            {
                {"minClusterPixels", getPrimitiveValidator<int>()},
                {"maxNumMaxima", getPrimitiveValidator<int>()},
                {"criticalAngleRads", getPrimitiveValidator<float>()},
                {"maxLineFitMSE", getPrimitiveValidator<float>()},
                {"minWhiteBlackDiff", getPrimitiveValidator<int>()},
                {"deglitch", getPrimitiveValidator<bool>()}
            },
            {},
            {}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }
}