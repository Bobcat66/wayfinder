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

#include "wfcore/fiducial/ArucoDetector.h"

namespace wf {
    ArucoDetector::ArucoDetector() {
        dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_APRILTAG_36h11);
        detectorParams = cv::aruco::DetectorParameters::create();
    }
    WFResult<std::vector<ApriltagDetection>> ArucoDetector::detect(const cv::Mat& im) {
        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners;
        cv::aruco::detectMarkers(im, dictionary, markerCorners, markerIds, detectorParams);
        std::vector<ApriltagDetection> detections;
        for (int i = 0; i < markerIds.size(); i++) {
            auto& corners = markerCorners[i];
            detections.emplace_back(
                markerIds[i],
                std::array<cv::Point2d, 4>{
                    cv::Point2d{corners[0].x,corners[0].y},
                    cv::Point2d{corners[1].x,corners[1].y},
                    cv::Point2d{corners[2].x,corners[2].y},
                    cv::Point2d{corners[3].x,corners[3].y}
                },
                0.0,
                0.0,
                "tag36h11"
            );
        }
        return WFResult<std::vector<ApriltagDetection>>::success(std::move(detections));
    }
}