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

#include "wfcore/common/json_utils.h"

namespace wf {
    using enum WFStatus;

    // Error messages should only provide the qualified name of the property which caused the error
    // To do this, each struct and map validator will prepend the name of the property where the fault occurred
    // to the error message
    // Array validators will prepend the index where the fault occurred
    WFStatusResult JSONStructValidator::operator()(const JSON& jobject) const {
        if (!jobject.is_object()) 
            return WFStatusResult::failure(JSON_INVALID_TYPE);

        for (const auto& [property,validator] : properties) {
            auto it = jobject.find(property);
            if (it == jobject.end()) {
                if (required.contains(property))
                    return WFStatusResult::failure(JSON_PROPERTY_NOT_FOUND,"/{}",property);
                continue;
            }
            auto res = (*validator)(it.value());
            if (!res)
                return res.hasMsg()
                    ? WFStatusResult::failure(res.status(),"/{}{}",property,res.what())
                    : WFStatusResult::failure(res.status(),"/{}",property);
        }
        return WFStatusResult::success();
    }

    WFStatusResult JSONMapValidator::operator()(const JSON& jobject) const {
        if (!jobject.is_object())
            return WFStatusResult::failure(JSON_INVALID_TYPE);
        
        for (const auto& [key,value] : jobject.items()) {
            if (!std::regex_match(key,keyMatcher))
                return WFStatusResult::failure(JSON_SCHEMA_VIOLATION,"/{}",key);
            auto res = (*valueValidator)(value);
            if (!res)
                return res.hasMsg()
                    ? WFStatusResult::failure(res.status(),"/{}{}",key,res.what())
                    : WFStatusResult::failure(res.status(),"/{}",key);
        }
        return WFStatusResult::success();
    }

    WFStatusResult JSONArrayValidator::operator()(const JSON& jobject) const {
        if (!jobject.is_array()) 
            return WFStatusResult::failure(JSON_INVALID_TYPE);

        size_t size = jobject.size();
        if (minSize > size || maxSize < size)
            return WFStatusResult::failure(JSON_SCHEMA_VIOLATION);

        for (size_t i = 0; i < size; ++i) {
            auto res = (*valueValidator)(jobject[i]);
            if (!res)
                return res.hasMsg()
                    ? WFStatusResult::failure(res.status(),"/{}{}",i,res.what())
                    : WFStatusResult::failure(res.status(),"/{}",i);
        }
        return WFStatusResult::success();
        
    }

    WFStatusResult JSONEnumValidator::operator()(const JSON& jobject) const {
        if (!jobject.is_string())
            return WFStatusResult::failure(JSON_INVALID_TYPE);
        
        if (!enumValues.contains(jobject.get<std::string>()))
            return WFStatusResult::failure(JSON_SCHEMA_VIOLATION);
        
        return WFStatusResult::success();
    }
}