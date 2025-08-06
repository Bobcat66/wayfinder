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
#include <functional>
#include <unordered_set>
#include <regex>
#include <limits>
#include "wfcore/common/jval_compat.h"

// TODO: Refactor JSON utilities.
// The way they are structured now, a lot of unnecessary checks are performed
namespace wf {

    constexpr size_t array_maxsize = std::numeric_limits<std::size_t>::max();

    // NOTE: In general, JSON code is not performance critical (hot path marshalling is done through WIPS) and should be written primarily for clarity and maintainability, not performance

    using JSON = nlohmann::json;

    inline loggerPtr& jsonLogger() { 
        static auto logger = LoggerManager::getInstance().getLogger("JSON");
        return logger;
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
    [[ deprecated ]]
    WFResult<T> jsonCast(const JSON& jobject, bool verbose) {
        return jsonCast<T>(jobject,"JSON Object",verbose);
    }

    // Returns a primitive optional property or a default
    template <typename T>
    inline T getJSONOpt(const JSON& jobject,std::string_view property, T defaultValue) {
        return jobject.contains(property) ? jobject[property].get<T>() : defaultValue;
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

        // getValidator_impl() should return a pointer to a persistent singleton validation functor
        static WFStatusResult validate(const JSON& jobject) {
            return JVResToWF((*DerivedType::getValidator_impl())(jobject));
        }

        std::string dump() const {
            auto jres = toJSON(static_cast<const DerivedType&>(*this));
            if (!jres) [[ unlikely ]] {
                throw json_error("Error while dumping json: {}",wfstatus_name_view(jres.status()));
            }
            return jres.value().dump();
        }
    };
}