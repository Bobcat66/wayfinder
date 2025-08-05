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

#include "wfcore/configuration/WFDefaults.h"
#include "wfdetail/validation/tag_detector_validators.h"
#include "wfdetail/validation/inference_validators.h"
#include "wfdetail/validation/video_validators.h"
#include "wfcore/common/json_utils.h"

namespace impl {
    using namespace wf;

    static const JSONValidationFunctor* getDefaultsValidator() {
        static JSONStructValidator validator(
            {
                {"engineType", detail::getInferenceEngineTypeValidator()},
                {"modelArch", detail::getModelArchValidator()},
                {"tensorParams", detail::getTensorParamsValidator()},
                {"nmsThreshold", getPrimitiveValidator<float>()},
                {"confidenceThreshold", getPrimitiveValidator<float>()},
                {"modelFile", getPrimitiveValidator<std::string>()},
                {"modelColorSpace", detail::getEncodingValidator()},
                {"qtps", detail::getDetectorQTPsValidator()},
                {"tagDetectorConfig", detail::getDetectorQTPsValidator()},
                {"tagSize", getPrimitiveValidator<double>()},
                {"tagFamily", getPrimitiveValidator<std::string>()},
                {"tagField", getPrimitiveValidator<std::string>()}
            },
            {},
            {}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }

}

namespace wf {
    WFStatusResult WFDefaults::load_impl(const JSON& jobject) {
        auto valres = (*impl::getDefaultsValidator())(jobject);
        if (!valres) return valres;
        
    }
}