/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of Wayfinder.
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

#include "wfcore/types.h"
#include "wfcore/fiducial/ApriltagDetection.h"
#include "wfcore/utils/geometry.h"
#include "wfcore/utils/units.h"

#include <gtsam/geometry/Rot2.h>

#include <unordered_map>
#include <vector>
#include <cmath>
#include <cstdint>

namespace wf {

    struct ApriltagDetectorConfig {
        int numThreads = 1;
        float quadDecimate = 2.0f;
        float quadSigma = 0.0f;
        bool refineEdges = true;
        double decodeSharpening = 0.25;
        bool debug = false;
    };

    struct QuadThresholdParams {
        int minClusterPixels = 300;
        int maxNumMaxima = 10;
        float criticalAngleRads = degreesToRadians(45.0f);
        float maxLineFitMSE = 10.0f;
        int minWhiteBlackDiff = 5;
        bool deglitch = false;
    };

    class ApriltagDetector {
    public:
        ApriltagDetector();
        ~ApriltagDetector();
        [[nodiscard]] std::vector<ApriltagDetection> detect(int width, int height, int stride, uint8_t* buf) const noexcept;
        inline std::vector<ApriltagDetection> detect(int width, int height, uint8_t* buf) const noexcept {
            return detect(width,height,width,buf);
        }
        inline std::vector<ApriltagDetection> detect(const cv::Mat& im) const noexcept {
            assert(im.type() == CV_8UC1); // Asserts that the matrix contains an 8 bit grayscale image
            return detect(im.rows,im.cols,im.step[0],im.data);
        };
        // Returns a copy of the QTPs
        QuadThresholdParams getQuadThresholdParams() const;
        // Returns a copy of the configs
        ApriltagDetectorConfig getConfig() const;
        void setQuadThresholdParams(const QuadThresholdParams params);
        void setConfig(const ApriltagDetectorConfig config);
        void addFamily(const std::string& familyName);
        void removeFamily(const std::string& familyName);
        void clearFamilies();
    private:
        std::unordered_map<std::string,void*> families;
        void* cdetector;
    };


}
