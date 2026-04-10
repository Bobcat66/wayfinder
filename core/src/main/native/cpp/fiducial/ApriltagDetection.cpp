/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 Jesse Kane
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

#include "wfcore/fiducial/ApriltagDetection.h"

namespace impl {

    // TODO: Figure out a better way to do this
    static std::unordered_map<std::string, wips_u8_t> tagFamilyIDs = {
        {"tag36h11", 0},
        {"tag36h10", 1},
        {"tag25h9", 2},
        {"tag16h5", 3},
        {"tagCircle21h7", 4},
        {"tagCircle49h12", 5},
        {"tagCustom48h12", 6},
        {"tagStandard41h12", 7},
        {"tagStandard52h13", 8}
    };

    static std::unordered_map<wips_u8_t, std::string> tagFamilyNames = {
        {0, "tag36h11"},
        {1, "tag36h10"},
        {2, "tag25h9"},
        {3, "tag16h5"},
        {4, "tagCircle21h7"},
        {5, "tagCircle49h12"},
        {6, "tagCustom48h12"},
        {7, "tagStandard41h12"},
        {8, "tagStandard52h13"}
    };
}

namespace wf {

    wips_apriltag_detection_t ApriltagDetection::toWIPS_impl(const ApriltagDetection& wfcore_object) {
        return {
            wfcore_object.id,
            wfcore_object.corners[0].x,wfcore_object.corners[0].y,
            wfcore_object.corners[1].x,wfcore_object.corners[1].y,
            wfcore_object.corners[2].x,wfcore_object.corners[2].y,
            wfcore_object.corners[3].x,wfcore_object.corners[3].y,
            wfcore_object.decisionMargin,
            wfcore_object.hammingDistance,
            impl::tagFamilyIDs.at(wfcore_object.family)
        };
    }
    
    ApriltagDetection ApriltagDetection::fromWIPS_impl(const wips_apriltag_detection_t& wips_struct) {
        return {
            wips_struct.fiducial_id,
            {
                cv::Point2d{wips_struct.corner0_x, wips_struct.corner0_y},
                cv::Point2d{wips_struct.corner1_x, wips_struct.corner1_y},
                cv::Point2d{wips_struct.corner2_x, wips_struct.corner2_y},
                cv::Point2d{wips_struct.corner3_x, wips_struct.corner3_y}
            },
            wips_struct.decision_margin,
            wips_struct.hamming_distance,
            impl::tagFamilyNames.at(wips_struct.tag_family_id)
        };
    }
}