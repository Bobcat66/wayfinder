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

namespace wf {

    // NOTE: In general, JSON code is not performance critical (hot path marshalling is done through WIPS) and should be written primarily for clarity and maintainability, not performance

    // JSON Status codes
    enum class JSONStatus {
        Ok,
        PropertyNotFound,
        InvalidType,
        SchemaViolation,
        Unknown
    };

    inline std::unordered_map<JSONStatus,std::string> JSONStatusStrings = {
        {JSONStatus::Ok,"Ok"},
        {JSONStatus::PropertyNotFound,"PropertyNotFound"},
        {JSONStatus::InvalidType,"InvalidType"},
        {JSONStatus::SchemaViolation,"SchemaViolation"},
        {JSONStatus::Unknown,"Unknown"}
    };

    using JSONObject = nlohmann::json;

    template <typename T>
    using JSONStatusResult = StatusfulResult<T, JSONStatus, JSONStatus::Ok>;

    inline loggerPtr& jsonLogger() { 
        static auto logger = LoggerManager::getInstance().getLogger("JSON");
        return logger;
    }

    // Returns whether all the properties listed are present in the jobject
    // optionally logs missing properties to the global logger, can also be
    // provided with an object name for clearer logs
    inline bool validateProperties(
        const JSONObject& jobject, 
        std::initializer_list<std::string_view> properties,
        std::string_view objectName = "JSON Object",
        bool verbose = true
    ) noexcept {
        bool valid = true;
        for (std::string_view property : properties) {
            if (!jobject.contains(property)) {
                if (verbose) jsonLogger()->warn("{} missing property '{}'",objectName,property);
                valid = false;
            }
        }
        return valid;
    }

    // Overload with default object name for conciseness
    inline bool validateProperties(
        const JSONObject& jobject,
        std::initializer_list<std::string_view> properties,
        bool verbose
    ) noexcept {
        return validateProperties(jobject,properties,"JSON Object",verbose);
    }

    // CRTP-based JSON serializable object interface
    template <typename DerivedType>
    class JSONSerializable {
    public:
        static JSONStatusResult<JSONObject> toJSON(const DerivedType& object) {
            return DerivedType::toJSON_impl(object);
        }
        static JSONStatusResult<DerivedType> fromJSON(const JSONObject& jobject) {
            return DerivedType::fromJSON_impl(jobject);
        }

        std::string dump() const {
            auto jres = toJSON(static_cast<const DerivedType&>(*this));
            if (!jres) [[ unlikely ]] {
                throw json_error("Error while dumping json: {}",JSONStatusStrings.at(jres.status()));
            }
            return jres.value().dump();
        }
    };
}