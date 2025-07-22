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

#include <nlohmann/json.hpp>
#include "wfcore/common/status.h"
#include <unordered_map>
#include "wfcore/common/logging.h"
#include "wfcore/common/wfexcept.h"

namespace wf {

    // NOTE: In general, JSON code is not performance critical (hot path marshalling is done through WIPS) and should be written primarily for clarity and maintainability, not performance

    using JSON = nlohmann::json;

    inline loggerPtr& jsonLogger() { 
        static auto logger = LoggerManager::getInstance().getLogger("JSON");
        return logger;
    }

    // Returns whether all the properties listed are present in the jobject
    // optionally logs missing properties to the global logger, can also be
    // provided with an object name for clearer logs
    inline WFStatusResult validateProperties(
        const JSON& jobject, 
        std::initializer_list<std::string_view> properties,
        std::string_view objectName = "JSON Object",
        bool verbose = true
    ) noexcept {
        if (!jobject.is_object()) {
            if (verbose) jsonLogger()->warn("{} is not an object",objectName);
            return WFStatusResult::failure(WFStatus::JSON_INVALID_TYPE);
        }
        bool valid = true;
        for (std::string_view property : properties) {
            if (!jobject.contains(property)) {
                if (verbose) jsonLogger()->warn("{} missing property '{}'",objectName,property);
                valid = false;
            }
        }
        return valid 
            ? WFStatusResult::success()
            : WFStatusResult::failure(WFStatus::JSON_PROPERTY_NOT_FOUND);
    }

    // Overload with default object name for conciseness
    inline WFStatusResult validateProperties(
        const JSON& jobject,
        std::initializer_list<std::string_view> properties,
        bool verbose
    ) noexcept {
        return validateProperties(jobject,properties,"JSON Object",verbose);
    }

    // CRTP-based JSON serializable object interface
    template <typename DerivedType>
    class JSONSerializable {
    public:
        static WFResult<JSON> toJSON(const DerivedType& object) {
            return DerivedType::toJSON_impl(object);
        }
        static WFResult<DerivedType> fromJSON(const JSON& jobject) {
            return DerivedType::fromJSON_impl(jobject);
        }

        std::string dump() const {
            auto jres = toJSON(static_cast<const DerivedType&>(*this));
            if (!jres) [[ unlikely ]] {
                throw json_error("Error while dumping json: {}",wfstatus_name_view(jres.status()));
            }
            return jres.value().dump();
        }
    };

    template <typename T>
    WFResult<T> getProperty(const JSON& jobject,std::string_view propertyName,std::string_view objectName = "JSON Object",bool verbose = true) {
        // Jobject verification
        if (!jobject.is_object()) {
            if (verbose)
                jsonLogger()->error("{} is not an object",objectName);
            return WFResult<T>::failure(WFStatus::JSON_INVALID_TYPE);
        }
        if (!jobject.contains(propertyName)) {
            if (verbose)
                jsonLogger()->error("property {} of {} not found",propertyName,objectName);
            return WFResult<T>::failure(WFStatus::JSON_PROPERTY_NOT_FOUND);
        }

        // Safely extract the field
        try {
            T property = jobject[propertyName].get<T>();
            return WFResult<T>::success(std::move(property));
        } catch (const JSON::type_error& e) {
            if (verbose)
                jsonLogger()->error("Type error while parsing property {} of {}: {}",propertyName,objectName,e.what());
            return WFResult<T>::failure(WFStatus::JSON_INVALID_TYPE);
        } catch (const JSON::exception& e) {
            if (verbose)
                jsonLogger()->error("JSON error while parsing property {} of {}: {}",propertyName,objectName,e.what());
            return WFResult<T>::failure(WFStatus::JSON_UNKNOWN);
        }
    }

    template <typename T>
    WFResult<T> getProperty(const JSON& jobject,std::string_view propertyName,bool verbose) {
        return getProperty<T>(jobject,propertyName,"JSON Object",verbose);
    }

    template <typename T>
    WFResult<T> jsonCast(const JSON& jobject, std::string_view objectName = "JSON Object", bool verbose = true) {
        try {
            T castVal = jobject.get<T>();
            return WFResult<T>::success(std::move(castVal));
        } catch (const JSON::type_error& e) {
            if (verbose) jsonLogger()->error("Type error while casting {}: {}", objectName, e.what());
            return WFResult<T>::failure(WFStatus::JSON_INVALID_TYPE);
        } catch (const JSON::exception& e) {
            if (verbose) jsonLogger()->error("JSON error while casting {}: {}", objectName, e.what());
            return WFResult<T>::failure(WFStatus::JSON_UNKNOWN);
        }
    }

    template <typename T>
    WFResult<T> jsonCast(const JSON& jobject, bool verbose) {
        return jsonCast<T>(jobject,"JSON Object",verbose);
    }
}