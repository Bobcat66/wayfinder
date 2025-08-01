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
        JSONStructValidator(
            std::unordered_map<std::string,const JSONValidationFunctor*> properties_,
            std::unordered_set<std::string> required_
        ) 
        : properties(std::move(properties_))
        , required(std::move(required_)) {}
        WFStatusResult operator()(const JSON& jobject) const override;
    private:
        const std::unordered_map<std::string,const JSONValidationFunctor*> properties;
        const std::unordered_set<std::string> required;
    };

    // Meant for validating objects with an indeterminate number of properties that share a type
    // (i.e. JSON objects that behave like maps)
    class JSONMapValidator : public JSONValidationFunctor {
    public:
        JSONMapValidator(const JSONValidationFunctor* valueValidator_,const std::string& keyPattern = R"(^.*$)") : valueValidator(valueValidator_), keyMatcher(keyPattern) {}
        WFStatusResult operator()(const JSON& jobject) const override;
    private:
        const std::regex keyMatcher;
        const JSONValidationFunctor* valueValidator;
    };

    class JSONEnumValidator : public JSONValidationFunctor {
    public:
        JSONEnumValidator(std::unordered_set<std::string> enumValues_) : enumValues(std::move(enumValues_)) {}
        WFStatusResult operator()(const JSON& jobject) const override;
    private:
        const std::unordered_set<std::string> enumValues;
    };

    class JSONArrayValidator : public JSONValidationFunctor {
    public:
        JSONArrayValidator(const JSONValidationFunctor* valueValidator_,size_t minSize_ = 0,size_t maxSize_ = array_maxsize)
        : valueValidator(valueValidator_), minSize(minSize_), maxSize(maxSize_) {}
        WFStatusResult operator()(const JSON& jobject) const override;
    private:
        const size_t minSize;
        const size_t maxSize;
        const JSONValidationFunctor* valueValidator;
    };

    class JSONUnionValidator : public JSONValidationFunctor {
    public:
        JSONUnionValidator(std::vector<JSONValidationFunctor*> validators_) : validators(std::move(validators_)) {}
        WFStatusResult operator()(const JSON& jobject) const override;
    private:
        const std::vector<JSONValidationFunctor*> validators;
    };

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