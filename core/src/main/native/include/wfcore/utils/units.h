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

#include "wfcore/utils/geometry.h"

#include <concepts>

namespace wf {

    template <std::floating_point T>
    constexpr inline T metersToInches(T meters) {
        return meters * 10000 / 254;
    }

    template <std::floating_point T>
    constexpr inline T inchesToMeters(T inches) {
        return inches * 254 / 10000;
    }

    template <std::floating_point T>
    constexpr inline T degreesToRadians(T degrees) {
        return degrees * static_cast<T>(constants::pi) / static_cast<T>(180);
    }

    template <std::floating_point T>
    constexpr inline T radiansToDegrees(T radians) {
        return radians * static_cast<T>(180) / static_cast<T>(constants::pi);
    }
}