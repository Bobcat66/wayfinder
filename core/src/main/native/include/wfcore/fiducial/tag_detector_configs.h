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

#include <cmath>
#include <cstdint>
#include <string>

namespace wf {

    struct ApriltagDetectorConfig {
        int numThreads = 1;
        float quadDecimate = 2.0f;
        float quadSigma = 0.0f;
        bool refineEdges = true;
        double decodeSharpening = 0.25;
        bool debug = false;
        std::string string() const;
        bool operator==(const ApriltagDetectorConfig&) const = default;
    };

    std::ostream& operator<<(std::ostream& os, const ApriltagDetectorConfig& config);

    struct QuadThresholdParams {
        int minClusterPixels = 5;
        int maxNumMaxima = 10;
        float criticalAngleRads = 0.0f;
        float maxLineFitMSE = 10.0f;
        int minWhiteBlackDiff = 5;
        bool deglitch = false;
        std::string string() const;
        bool operator==(const QuadThresholdParams&) const = default;
    };

    std::ostream& operator<<(std::ostream& os, const QuadThresholdParams& qtps);
}