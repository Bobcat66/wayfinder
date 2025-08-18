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
#include <concepts>
#include "wfcore/common/wfdef.h"
#include "jval/jvruntime.hpp"
#include "wfcore/common/jval_compat.h"
#include <utility>
#include <memory>

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
    using ValidatorDecoder = std::pair<const jval::JSONValidationFunctor*,std::function<WFResult<T>(const JSON&)>>;

    template <typename... Ts>
    class JSONVariantDecoder {
    public:
        JSONVariantDecoder(ValidatorDecoder<Ts>... valdecs) {
            (addValDec(std::forward<ValidatorDecoder<Ts>>(valdecs)), ...);
        }
        WFResult<std::variant<Ts...>> decode(const JSON& jobject) {
            for (auto& valdec : valdecsvec) {
                if (!(*valdec.first)(jobject)) continue;
                // We will propagate forward the first schema that is validated
                // Errors will be propagated transparently
                return (valdec.second)(jobject);
            }
            return WFResult<std::variant<Ts...>>::failure(
                WFStatus::JSON_SCHEMA_VIOLATION,
                "Value does not match any variant schema"
            );
        }
    private:
        template <typename T>
        void addValDec(ValidatorDecoder<T> valdec) {
            auto validator = valdec.first;
            auto decoder   = std::move(valdec.second);
            // Constructs a new dynamically allocated ValDec, as we need a level of indirection to store the valdecs in a vector
            valdecsvec.emplace_back(
                validator,
                [decoder = std::move(decoder)](const JSON& jobject) -> WFResult<std::variant<Ts...>> {
                    auto res = decoder(jobject);
                    if (!res) return WFResult<std::variant<Ts...>>::propagateFail(res);
                    return WFResult<std::variant<Ts...>>::success(
                        std::variant<Ts...>(std::move(res.value()))
                    );
                }
            );
        }
        std::vector<ValidatorDecoder<std::variant<Ts...>>> valdecsvec;
    };

    // CTAD for JSONVariantDecoder
    template <typename... Ts>
    JSONVariantDecoder(ValidatorDecoder<Ts>...) -> JSONVariantDecoder<Ts...>;

    template <typename T>
    [[ deprecated ]]
    WFResult<T> jsonCast(const JSON& jobject, bool verbose) {
        return jsonCast<T>(jobject,"JSON Object",verbose);
    }

    // Returns an optional property or a default
    template <typename T>
    inline T getJSONOpt(const JSON& jobject,std::string_view property, T defaultValue) {
        return jobject.contains(property) ? jobject[property].get<T>() : defaultValue;
    }

    // CRTP-based JSON serializable object interface
    template <typename Derived>
    class JSONSerializable {
    public:
        static WFResult<JSON> toJSON(const Derived& object) {
            return Derived::toJSON_impl(object);
        }
        static WFResult<Derived> fromJSON(const JSON& jobject) {
            return Derived::fromJSON_impl(jobject);
        }

        // getValidator_impl() should return a pointer to a persistent singleton validation functor
        static WFStatusResult validate(const JSON& jobject) {
            return JVResToWF((*Derived::getValidator_impl())(jobject));
        }

        std::string dump() const {
            auto jres = toJSON(static_cast<const Derived&>(*this));
            if (!jres) [[ unlikely ]] {
                throw json_error("Error while dumping json: {}",wfstatus_name_view(jres.status()));
            }
            return jres.value().dump();
        }
    };

    // For Nlohmann JSON integration
    template <typename Derived>
    void to_json(JSON& j, const Derived& value)
    requires std::derived_from<Derived, JSONSerializable<Derived>>
    {
        auto res = Derived::toJSON(value);
        if (!res)
            throw wf_result_error(res);
        j = std::move(res.value());
    }

    template <typename Derived>
    void from_json(const JSON& j, Derived& value)
    requires std::derived_from<Derived, JSONSerializable<Derived>>
    {
        auto res = Derived::fromJSON(j);
        if (!res)
            throw wf_result_error(res);
        value = std::move(res.value());
    }
}