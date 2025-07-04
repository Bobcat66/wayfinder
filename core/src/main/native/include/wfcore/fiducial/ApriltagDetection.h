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

#include <opencv2/core/types.hpp>

#include <vector>
#include <string>
#include <array>

namespace wf {

    struct ApriltagDetection {
        int id;
        std::array<cv::Point2d, 4> corners;
        double decisionMargin;
        double hammingDistance;
        std::string family;
        ApriltagDetection(
            int id_, 
            std::array<cv::Point2d, 4> corners_, 
            double decisionMargin_, double hammingDistance_,
            std::string family_
        ) : id(id_), 
            corners(std::move(corners_)),
            decisionMargin(decisionMargin_), hammingDistance(hammingDistance_),
            family(std::move(family_)) {}
    };
    
}