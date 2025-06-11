// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "types.h"
#include "apriltag/AprilTagDetection.h"
#include "utils/geometry.h"
#include "utils/units.h"

#include <gtsam/geometry/Rot2.h>

#include <unordered_map>
#include <vector>
#include <cmath>
#include <cstdint>

namespace wf {

    struct AprilTagDetectorConfig {
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

    class AprilTagDetector {
        std::vector<void*> families;
        void* cdetector;

        public:
        AprilTagDetector();
        ~AprilTagDetector();
        std::vector<AprilTagDetection> detect(int width, int height, int stride, uint8_t* buf) const;
        std::vector<AprilTagDetection> detect(int width, int height, uint8_t* buf) const {
            return detect(width,height,width,buf);
        }
        std::vector<AprilTagDetection> detect(const cv::Mat& im) const {
            assert(im.type() == CV_8UC1); // Asserts that the matrix contains an 8 bit grayscale image
            return detect(im.rows,im.cols,im.step[0],im.data);
        };
        // Returns a copy of the QTPs
        QuadThresholdParams getQuadThresholdParams() const;
        // Returns a copy of the configs
        AprilTagDetectorConfig getConfig() const;
        void setQuadThresholdParams(const QuadThresholdParams);
        void setConfig(const AprilTagDetectorConfig);
        void addFamily(const std::string& familyName);
        void removeFamily(const std::string& familyName);
        void clearFamilies();
    };


}
