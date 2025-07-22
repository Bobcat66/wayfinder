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

#include "wfcore/common/status/StatusfulObject.h"
#include "wfcore/common/logging.h"
#include <variant>
#include <type_traits>
#include <functional>

namespace wf {
    // StatusfulResult is a monad which wraps a value and a status code
    // A monad is a design pattern that wraps a value along with some extra context,
    // such as success/failure, optionality, or side effects. It provides a way to 
    // chain operations on the wrapped value, automatically handling the context 
    // (like errors or missing values) so that the code stays clean and easy to read.
    template <typename T,status_code status_type, status_type nominal_status>
    class StatusfulResult {
    public:
        constexpr StatusfulResult(status_type status, std::optional<T> val = std::nullopt)
        : status_(status), optval(std::move(val)) {}

        constexpr bool ok() const { 
            return status_ == nominal_status && optval.has_value(); 
        }

        constexpr explicit operator bool() const {
            return ok();
        }

        // Calling value() on a non-nominal statusful result will throw
        constexpr const T& value() const {
            return optval.value();
        }

        // Calling value() on a non-nominal statusful result will throw
        constexpr T& value() {
            return optval.value();
        }

        constexpr status_type status() const {
            return status_;
        }

        // Applies mapper to the wrapped value, and returns a StatusfulResult wrapping the result
        template <typename F>
        constexpr auto and_then(F&& mapper) const -> decltype(mapper(std::declval<T>())) {
            using ResultType = decltype(mapper(std::declval<T>()));
            static_assert(std::is_same_v<typename ResultType::status_type, status_type>, 
                "Mapper must return StatusfulResult with same status_type");
            static_assert(ResultType::nominal_status == nominal_status, 
                "Mapper must use same nominal_status");
            if (!ok()) return ResultType::failure(status());
            return mapper(optval.value());
        }

        // and_then() for nerds
        template <typename F>
        constexpr auto bind(F&& mapper) const -> decltype(mapper(std::declval<T>())) {
            return and_then(mapper);
        }

        static constexpr StatusfulResult success(T value) {
            return {nominal_status, std::move(value)};
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

        static constexpr StatusfulResult failure(status_type code) {
            return { code, std::nullopt };
        }

        // Propagates a statusful result. If the statusfulResult passed in is nominal, this method will
        // return another nominal statusfulResult that wraps valueOnSuccess
        template <typename U>
        static constexpr StatusfulResult propagate(const StatusfulResult<U,status_type,nominal_status>& result, T&& valueOnSuccess) {
            if (result) {
                return success(std::move(valueOnSuccess));
            } else {
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
        const status_type status_;
        std::optional<T> optval;
    };

    template <status_code status_type, status_type nominal_status>
    using StatusResult = StatusfulResult<std::monostate,status_type,nominal_status>;
}