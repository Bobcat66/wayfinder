// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "types.h"
#include "utils/geometry.h"
#include "utils/units.h"

#include <apriltag.h>
#include <gtsam/geometry/Rot2.h>

#include <unordered_map>
#include <vector>
#include <cmath>

namespace wf {

    typedef apriltag_family_t* (*apriltag_family_creator)();
    typedef void (*apriltag_family_destructor)(apriltag_family_t*);
    
    enum TagFamily {
        TAG_36H11,
        TAG_36H10,
        TAG_25H9,
        TAG_16H5,
        TAG_CIRCLE_21H7,
        TAG_CIRCLE_49H12,
        TAG_CUSTOM_48H12,
        TAG_STANDARD_41H12,
        TAG_STANDARD_52H13,
        N_FAMILIES // Number of defined families
    };

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
        double criticalAngleRads = degreesToRadians(45.0);
        float maxLineFitMSE = 10.0f;
        int minWhiteBlackDiff = 5;
        bool deglitch = false;
    };

    class ApriltagDetector {
        const QuadThresholdParams qtps;
        const ApriltagDetectorConfig config;
        std::unordered_map<int,apriltag_family_t*> apriltagFamilies;
        apriltag_detector_t* cdetector;

        public:
        ApriltagDetector();
        ~ApriltagDetector();
        std::vector<AprilTagObservation> detect(const cv::Mat& image) const;
        const QuadThresholdParams& getQuadThresholdParams() const;
        const ApriltagDetectorConfig& getConfig() const;
        void setQuadThresholdParams(const QuadThresholdParams);
        void setConfig(const ApriltagDetectorConfig);
        void addFamily(TagFamily family);
        void removeFamily(TagFamily family);
        void clearFamilies();
    };


}
