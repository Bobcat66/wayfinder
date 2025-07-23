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
#include <unordered_map>
#include <functional>

// TODO: Refactor JSON utilities.
// The way they are structured now, a lot of unnecessary checks are performed
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
    // [[ deprecated ]]
    inline WFStatusResult validateProperties(
        const JSON& jobject, 
        std::initializer_list<std::string_view> properties,
        std::string_view objectName = "JSON Object",
        bool verbose = true
    ) noexcept {
        static thread_local std::vector<std::string> missingProps;
        if (!jobject.is_object()) {
            if (verbose) jsonLogger()->warn("{} is not an object",objectName);
            return WFStatusResult::failure(WFStatus::JSON_INVALID_TYPE);
        }
        missingProps.clear();
        bool valid = true;
        for (std::string_view property : properties) {
            if (!jobject.contains(property)) {
                missingProps.emplace_back(property);
                valid = false;
            }
        }
        if (!valid) {
            std::string msg = std::format("{} missing properties: ",objectName);
            for (const auto& missingProp : missingProps) {
                msg += std::format(" {}",missingProp);
            }
            if (verbose) jsonLogger()->error(msg);
            return WFStatusResult::failure(
                WFStatus::JSON_PROPERTY_NOT_FOUND,
                std::move(msg)
            );
        }
        return WFStatusResult::success();
    }

    // Overload with default object name for conciseness
    // [[ deprecated ]]
    inline WFStatusResult validateProperties(
        const JSON& jobject,
        std::initializer_list<std::string_view> properties,
        bool verbose
    ) noexcept {
        return validateProperties(jobject,properties,"JSON Object",verbose);
    }

    template <typename T>
    WFResult<T> getProperty(const JSON& jobject,std::string_view propertyName,std::string_view objectName = "JSON Object",bool verbose = true) {
        // Jobject verification
        if (!jobject.is_object()) {
            if (verbose)
                jsonLogger()->error("{} is not an object",objectName);
            return WFResult<T>::failure(WFStatus::JSON_INVALID_TYPE,"{} is not an object",objectName);
        }
        if (!jobject.contains(propertyName)) {
            if (verbose)
                jsonLogger()->error("property {} of {} not found",propertyName,objectName);
            return WFResult<T>::failure(WFStatus::JSON_PROPERTY_NOT_FOUND,"property {} of {} not found",propertyName,objectName);
        }

        // Safely extract the field
        try {
            T property = jobject[propertyName].get<T>();
            return WFResult<T>::success(std::move(property));
        } catch (const JSON::type_error& e) {
            if (verbose)
                jsonLogger()->error("Type error while parsing property {} of {}: {}",propertyName,objectName,e.what());
            return WFResult<T>::failure(WFStatus::JSON_INVALID_TYPE,"Type error while parsing property {} of {}: {}",propertyName,objectName,e.what());
        } catch (const JSON::exception& e) {
            if (verbose)
                jsonLogger()->error("JSON error while parsing property {} of {}: {}",propertyName,objectName,e.what());
            return WFResult<T>::failure(WFStatus::JSON_UNKNOWN,"JSON error while parsing property {} of {}: {}",propertyName,objectName,e.what());
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

    // An interface for objects that implement JSON validation
    // ALL JSONValidationFunctor objects should be STATIC objects with "eternal" lifetime
    // Additionally, calling the functors should be STRICTLY read-only
    class JSONValidationFunctor {
    public:
        virtual WFStatusResult operator()(const JSON& jobject) const = 0;
    };

    // Meant for validating primitive types (number, string, etc.)
    template <typename T>
    class JSONPrimitiveValidator : public JSONValidationFunctor {
    public:
        WFStatusResult operator()(const JSON& jobject) const override {
            try {
                jobject.get<T>();
                return WFStatusResult::success();
            } catch (const JSON::type_error& e) {
                return WFStatusResult::failure(WFStatus::JSON_INVALID_TYPE);
            } catch (const JSON::exception& e) {
                return WFStatusResult::failure(WFStatus::JSON_UNKNOWN);
            }
        }
    };

    template <typename T>
    const JSONPrimitiveValidator<T>* getPrimitiveValidator() {
        static JSONPrimitiveValidator<T> validator;
        return &validator;
    }

    // Meant for validating JSON objects with explicitly defined and typed properties
    // (i.e. JSON objects that behave like structs)
    class JSONStructValidator : public JSONValidationFunctor {
    public:
        JSONStructValidator(std::unordered_map<std::string,const JSONValidationFunctor*> properties_) : properties(std::move(properties_)) {}
        WFStatusResult operator()(const JSON& jobject) const override;
    private:
        const std::unordered_map<std::string,const JSONValidationFunctor*> properties;
    };

    // Meant for validating objects with an indeterminate number of properties that share a type
    // (i.e. JSON objects that behave like maps)
    class JSONMapValidator : public JSONValidationFunctor {
    public:
        JSONMapValidator(const JSONValidationFunctor* valueValidator_) : valueValidator(valueValidator_) {}
        WFStatusResult operator()(const JSON& jobject) const override;
    private:
        const JSONValidationFunctor* valueValidator;
    };

    class JSONArrayValidator : public JSONValidationFunctor {
    public:
        JSONArrayValidator(const JSONValidationFunctor* valueValidator_) : valueValidator(valueValidator_) {}
        WFStatusResult operator()(const JSON& jobject) const override;
    private:
        const JSONValidationFunctor* valueValidator;
    };

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
        static const JSONValidationFunctor* getValidator() {
            return DerivedType::getValidator_impl();
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