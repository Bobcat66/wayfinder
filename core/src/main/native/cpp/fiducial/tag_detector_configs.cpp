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

#include "wfcore/fiducial/tag_detector_configs.h"
#include <format>

namespace wf {
    std::string ApriltagDetectorConfig::string() const {
        return std::format(
            "{{{}, {}, {}, {}, {}, {}}}",
            numThreads,
            quadDecimate,
            quadSigma,
            refineEdges,
            decodeSharpening,
            debug
        );
    }

    std::ostream& operator<<(std::ostream& os, const ApriltagDetectorConfig& config) {
        os << config.string();
        return os;
    }
    
    std::string QuadThresholdParams::string() const {
        return std::format(
            "{{{}, {}, {}, {}, {}, {}}}",
            minClusterPixels,
            maxNumMaxima,
            criticalAngleRads,
            maxLineFitMSE,
            minWhiteBlackDiff,
            deglitch
        );
    }

    std::ostream& operator<<(std::ostream& os, const QuadThresholdParams& qtps) {
        os << qtps.string();
        return os;
    }
}