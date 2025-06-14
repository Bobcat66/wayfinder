// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: GPL-3.0-or-later

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