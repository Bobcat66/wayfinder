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

#include "wfcore/common/status.h"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include "wfcore/fiducial/ApriltagDetection.h"
// This is a prototype for aruco detection, for testing. It is not going to be part of the main pipeline at the moment

namespace wf {
    class ArucoDetector {
    public:
        ArucoDetector();
        WFResult<std::vector<ApriltagDetection>> detect(const cv::Mat& im);
    private:
        cv::Ptr<cv::aruco::Dictionary> dictionary;
        cv::Ptr<cv::aruco::DetectorParameters> detectorParams;
    };
}