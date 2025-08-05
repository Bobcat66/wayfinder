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
#include "wfcore/fiducial/tag_detector_configs.h"

namespace wf {

    class ApriltagDetector {
    public:
        ApriltagDetector();
        ~ApriltagDetector();
        [[nodiscard]]
        WFResult<std::vector<ApriltagDetection>> detect(int width, int height, int stride, uint8_t* buf) const noexcept;
        WFResult<std::vector<ApriltagDetection>> detect(int width, int height, uint8_t* buf) const noexcept {
            return detect(width,height,width,buf);
        }
        WFResult<std::vector<ApriltagDetection>> detect(const cv::Mat& im) const noexcept {
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
        WFStatusResult addFamily(const std::string& familyName) noexcept;
        [[ nodiscard ]]
        WFStatusResult removeFamily(const std::string& familyName) noexcept;
        void clearFamilies();
    private:
        std::unordered_map<std::string,void*> families;
        void* detectorHandle_;
    };


}
