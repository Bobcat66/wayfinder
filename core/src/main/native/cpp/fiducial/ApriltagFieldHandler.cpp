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

#include "wfcore/fiducial/ApriltagFieldHandler.h"

namespace wf {
    WFStatusResult ApriltagFieldHandler::loadField(std::string_view newFieldName) {
        std::string newFieldNameStr(newFieldName);
        auto jresult = resourceManager.loadResourceJSON(
            "fields",
            newFieldNameStr
        );
        if (!jresult)
            return WFStatusResult::propagateFail(jresult);
        
        auto fresult = ApriltagField::fromJSON(jresult.value());
        if (!fresult)
            return WFStatusResult::propagateFail(fresult);

        field = std::move(fresult.value());
        fieldName = std::move(newFieldNameStr);

        return WFStatusResult::success();
    }
}