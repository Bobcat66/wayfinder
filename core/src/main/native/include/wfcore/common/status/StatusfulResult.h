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

#include "wfcore/common/status/StatusType.h"
#include "wfcore/common/logging.h"
#include <cassert>
#include <variant>
#include <type_traits>
#include <functional>
#include <format>

namespace wf {
    
    // StatusfulResult is a monad which wraps a value and a status code
    // A monad is a design pattern that wraps a value along with some extra context,
    // such as success/failure, optionality, or side effects. It provides a way to 
    // chain operations on the wrapped value, automatically handling the context 
    // (like errors or missing values) so that the code stays clean and easy to read.
    template <typename T,status_code status_type, status_type nominal_status, const char* (*StringMapper) (status_type)>
    class StatusfulResult {
    public:
        constexpr StatusfulResult(status_type status, std::optional<T> val = std::nullopt) noexcept
        : status_(status), optval(std::move(val)), msg_(std::nullopt) {}

        StatusfulResult(status_type status, std::string msg, std::optional<T> val = std::nullopt) noexcept
        : status_(status), optval(std::move(val)), msg_(std::move(msg)) {}

        constexpr bool ok() const noexcept { 
            return status_ == nominal_status && optval.has_value(); 
        }

        constexpr bool hasMsg() const noexcept {
            return msg_.has_value();
        }

        constexpr explicit operator bool() const noexcept {
            return ok();
        }

        // Calling value() on a non-nominal statusful result is UB
        constexpr const T& value() const noexcept {
            assert(ok())
            return optval.value();
        }

        // Calling value() on a non-nominal statusful result is UB
        constexpr T& value() noexcept {
            assert(ok());
            return optval.value();
        }

        constexpr status_type status() const noexcept {
            return status_;
        }

        // Applies mapper to the wrapped value, and returns a StatusfulResult wrapping the result
        template <typename F>
        auto and_then(F&& mapper) const -> decltype(mapper(std::declval<T>())) {
            using ResultType = decltype(mapper(std::declval<T>()));
            static_assert(std::is_same_v<typename ResultType::status_type, status_type>, 
                "Mapper must return StatusfulResult with same status_type");
            static_assert(ResultType::nominal_status == nominal_status, 
                "Mapper must use same nominal_status");
            static_assert(ResultType::StringMapper == StringMapper,
                "Mapper must use same StringMapper");
            if (!ok()){
                if (hasMsg()) return ResultType::failure(status(),what());
                return ResultType::failure(status());
            }
            return mapper(optval.value());
        }
        
        // Returns the message string if present, lazily constructs a default if not
        std::string_view what() const noexcept {
            if (msg_.has_value()) return msg_.value();
            try {
                return ok() ? nominal_msg() : StringMapper(status());
            } catch (...) {
                return msgerr_unknown();
            }
        }

        // Ensures compatibility with the C ABI. For pure C++, what() is preferred.
        const char* c_what() const noexcept {
            if (msg_.has_value()) return msg_.value().c_str();
            try {
                return ok() ? nominal_msg() : StringMapper(status());
            } catch (...) {
                return msgerr_unknown();
            }
        }

        static constexpr StatusfulResult success(T value) {
            return { nominal_status, std::optional<T>(std::move(value)) };
        }

        // This will default-construct a result value
        // It is only recommended for use when the value type is std::monostate
        static constexpr StatusfulResult success() {
            return { nominal_status, std::optional<T>(std::in_place) };
        }

        template <typename... Args>
        static constexpr StatusfulResult success(std::in_place_t, Args&&... args) {
            return { nominal_status, std::optional<T>(std::in_place,std::forward<Args>(args)...) };     
        }

        static constexpr StatusfulResult failure(status_type code) noexcept {
            return { code, std::nullopt };
        }

        template <typename... Args>
        static StatusfulResult failure(status_type code,std::string_view fmt,Args&&... args) noexcept {
            try {
                return { 
                    code,
                    std::vformat(fmt,std::make_format_args(args...)),
                    std::nullopt
                };
            } catch (const std::format_error&) {
                return {
                    code,
                    msgerr_bad_format(),
                    std::nullopt
                };
            } catch (const std::bad_alloc&) {
                return {
                    code,
                    msgerr_bad_alloc(),
                    std::nullopt
                };
            }
        }

        // Propagates a statusful result. If the statusfulResult passed in is nominal, this method will
        // return another nominal statusfulResult that wraps valueOnSuccess
        template <typename U>
        static StatusfulResult propagate(const StatusfulResult<U,status_type,nominal_status,StringMapper>& result, T&& valueOnSuccess) noexcept {
            if (result) {
                return success(std::move(valueOnSuccess));
            } else {
                if (result.hasMsg()) return failure(result.status(),result.what());
                return failure(result.status());
            }
        }

        // More ergonomic syntactic sugar for cases where we don't care what gets propagated on success
        // It will return a default-constructed value on success
        template <typename U>
        static StatusfulResult propagateFail(const StatusfulResult<U,status_type,nominal_status,StringMapper>& result) noexcept {
            if (result) {
                // This should never happen
                return failure(result.status());
            } else {
                if (result.hasMsg()) return failure(result.status(),result.what());
                return failure(result.status());
            }
        }

        friend constexpr bool operator==(const StatusfulResult& lhs, const StatusfulResult& rhs) noexcept {
            return lhs.status_ == rhs.status_ && lhs.optval == rhs.optval;
        }

        friend constexpr bool operator!=(const StatusfulResult& lhs, const StatusfulResult& rhs) noexcept {
            return !(lhs == rhs);
        }
    private:
        static constexpr const char* nominal_msg() noexcept {
            return "Nominal";
        }
        static constexpr const char* msgerr_unknown() noexcept {
            return "MSGERR unknown";
        }
        static constexpr const char* msgerr_bad_alloc() noexcept {
            return "MSGERR bad_alloc";
        }
        static constexpr const char* msgerr_bad_format() noexcept {
            return "MSGERR bad_format";
        }

        const status_type status_;
        std::optional<T> optval;
        // Messages are only meaningful when the status is NOT NOMINAL!
        const std::optional<std::string> msg_;
    };

    template <status_code status_type, status_type nominal_status, const char* (*StringMapper) (status_type)>
    using StatusResult = StatusfulResult<std::monostate,status_type,nominal_status,StringMapper>;

}