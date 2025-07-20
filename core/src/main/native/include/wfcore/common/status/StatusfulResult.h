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

namespace wf {
    // This is for consistency with the StatusfulObject-based error system in non-object oriented contexts
    template <typename T,status_code U, U nominal_status>
    class StatusfulResult {
    public:
        constexpr StatusfulResult(U status, std::optional<T> val = std::nullopt)
        : status_(status), optval(std::move(val)) {}

        constexpr bool ok() const { return status_ == nominal_status && optval.has_value(); }

        constexpr explicit operator bool() const { return ok(); }

        constexpr const T& value() const { return optval.value(); }

        constexpr T& value() { return optval.value(); }

        constexpr U status() const { return status_; }

        static constexpr StatusfulResult success(T value) {
            return { nominal_status, std::move(value) };
        }

        template <typename... Args>
        static constexpr StatusfulResult success(std::in_place_t, Args&&... args) {
            return { nominal_status, std::optional<T>(std::in_place,std::forward<Args>(args)...) };
        }

        static constexpr StatusfulResult failure(U code) {
            return { code, std::nullopt };
        }

        friend constexpr bool operator==(const StatusfulResult& lhs, const StatusfulResult& rhs) noexcept {
            return lhs.status_ == rhs.status_ && lhs.optval == rhs.optval;
        }

        friend constexpr bool operator!=(const StatusfulResult& lhs, const StatusfulResult& rhs) noexcept {
            return !(lhs == rhs);
        }
    private:
        const U status_;
        std::optional<T> optval;
    };
}