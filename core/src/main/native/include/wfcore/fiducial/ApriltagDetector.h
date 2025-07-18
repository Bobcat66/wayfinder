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

#include "wfcore/types.h"
#include "wfcore/fiducial/ApriltagDetection.h"
#include "wfcore/utils/geometry.h"
#include "wfcore/utils/units.h"
#include "wfcore/common/status/StatusfulObject.h"

#include <gtsam/geometry/Rot2.h>

#include <unordered_map>
#include <vector>
#include <cmath>
#include <cstdint>
#include <string>
#include <optional>

namespace wf {

    enum class ApriltagDetectorStatus {
        Ok,
        InvalidFamily,
        NullFamily,
        NullDetector
    };

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

    class ApriltagDetector : public StatusfulObject<ApriltagDetectorStatus,ApriltagDetectorStatus::Ok> {
    public:
        ApriltagDetector();
        ~ApriltagDetector();
        [[nodiscard]]
        std::vector<ApriltagDetection> detect(int width, int height, int stride, uint8_t* buf) const noexcept;
        std::vector<ApriltagDetection> detect(int width, int height, uint8_t* buf) const noexcept {
            return detect(width,height,width,buf);
        }
        std::vector<ApriltagDetection> detect(const cv::Mat& im) const noexcept {
            assert(im.type() == CV_8UC1); // Asserts that the matrix contains an 8 bit grayscale image
            return detect(im.cols,im.rows,im.step[0],im.data);
        };
        // Returns a copy of the QTPs
        QuadThresholdParams getQuadThresholdParams() const noexcept;
        // Returns a copy of the configs
        ApriltagDetectorConfig getConfig() const noexcept;
        void setQuadThresholdParams(const QuadThresholdParams& params) noexcept;
        void setConfig(const ApriltagDetectorConfig& config) noexcept;
        [[ nodiscard ]]
        bool addFamily(const std::string& familyName) noexcept;
        [[ nodiscard ]]
        bool removeFamily(const std::string& familyName) noexcept;
        void clearFamilies();
    private:
        std::unordered_map<std::string,void*> families;
        void* detectorHandle_;
    };


}
