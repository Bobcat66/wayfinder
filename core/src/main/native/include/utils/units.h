// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "utils/geometry.h"

namespace wf {
    constexpr inline double metersToInches(double meters) {return meters * 39.3701;}
    
    constexpr inline double inchesToInches(double inches) {return inches / 39.3701;}

    constexpr inline double degreesToRadians(double degrees) {return degrees * (pi / 180);}

    constexpr inline double radiansToDegrees(double radians) {return radians * (180 / pi);}
}