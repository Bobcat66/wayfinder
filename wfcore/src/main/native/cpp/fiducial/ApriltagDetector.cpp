// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#include "fiducial/ApriltagDetector.h"

#include <apriltag.h>
#include <tag36h11.h>
#include <tag36h10.h>
#include <tag25h9.h>
#include <tag16h5.h>
#include <tagCircle21h7.h>
#include <tagCircle49h12.h>
#include <tagCustom48h12.h>
#include <tagStandard41h12.h>
#include <tagStandard52h13.h>

#include <map>
#include <unordered_map>

#define C_FAMILY(x) static_cast<apriltag_family_t*>(x)
#define C_DETECTOR static_cast<apriltag_detector_t*>(cdetector)

using namespace wf;

typedef apriltag_family_t* (*apriltag_family_creator)();
typedef void (*apriltag_family_destructor)(apriltag_family_t*);

static const std::unordered_map<std::string,apriltag_family_creator> family_creators = {
    {"tag36h11",tag36h11_create},
    {"tag36h10",tag36h10_create},
    {"tag25h9",tag25h9_create},
    {"tag16h5",tag16h5_create},
    {"tagCircle21h7",tagCircle21h7_create},
    {"tagCircle49h12",tagCircle49h12_create},
    {"tagCustom48h12",tagCustom48h12_create},
    {"tagStandard41h12",tagStandard41h12_create},
    {"tagStandard52h13",tagStandard52h13_create}
};

static const std::unordered_map<std::string,apriltag_family_destructor> family_destructors = {
    {"tag36h11",tag36h11_destroy},
    {"tag36h10",tag36h10_destroy},
    {"tag25h9",tag25h9_destroy},
    {"tag16h5",tag16h5_destroy},
    {"tagCircle21h7",tagCircle21h7_destroy},
    {"tagCircle49h12",tagCircle49h12_destroy},
    {"tagCustom48h12",tagCustom48h12_destroy},
    {"tagStandard41h12",tagStandard41h12_destroy},
    {"tagStandard52h13",tagStandard52h13_destroy}
};


ApriltagDetector::ApriltagDetector() {
    cdetector = apriltag_detector_create();
}

ApriltagDetector::~ApriltagDetector() {
    // Destroy apriltag detector
    apriltag_detector_destroy(C_DETECTOR);

    // Destroy apriltag families
    for (auto family : families){
        family_destructors.at({C_FAMILY(family)->name})(C_FAMILY(family));
    }
}

std::vector<ApriltagDetection> ApriltagDetector::detect(int height, int width, int stride, uint8_t* buf) const {
    image_u8_t im = {height,width,stride,buf};

    // Perform detection, returns a zarray of results
    auto rawDetections = apriltag_detector_detect(C_DETECTOR,&im);

    // Create output vector, preallocate memory to avoid reallocation costs
    std::vector<wf::ApriltagDetection> detections;
    detections.reserve(zarray_size(rawDetections));

    // Destructively converts rawDetections into a vector of wf::ApriltagDetections
    for (int i = 0; i < zarray_size(rawDetections); i++) {
        apriltag_detection_t* det;
        zarray_get(rawDetections, i, &det);
        detections.emplace_back(
            det->id,
            std::vector<cv::Point2d>{
                {det->p[0][0],det->p[0][1]},
                {det->p[1][0],det->p[1][1]},
                {det->p[2][0],det->p[2][1]},
                {det->p[3][0],det->p[3][1]}
            },
            det->decision_margin,
            det->hamming,
            det->family->name
        );
        apriltag_detection_destroy(det);
    }
    zarray_destroy(rawDetections);

    return detections;
}

QuadThresholdParams ApriltagDetector::getQuadThresholdParams() const {
    auto qtps = C_DETECTOR->qtp;
    return {
        .minClusterPixels = qtps.min_cluster_pixels,
        .maxNumMaxima = qtps.max_nmaxima,
        .criticalAngleRads = qtps.critical_rad,
        .maxLineFitMSE = qtps.max_line_fit_mse,
        .minWhiteBlackDiff = qtps.min_white_black_diff,
        .deglitch = static_cast<bool>(qtps.deglitch)
    };
}

ApriltagDetectorConfig ApriltagDetector::getConfig() const {
    return {
        .numThreads = C_DETECTOR->nthreads,
        .quadDecimate = C_DETECTOR->quad_decimate,
        .quadSigma = C_DETECTOR->quad_sigma,
        .refineEdges = C_DETECTOR->refine_edges,
        .decodeSharpening = C_DETECTOR->decode_sharpening,
        .debug = C_DETECTOR->debug
    };
}

void ApriltagDetector::addFamily(const std::string& familyName){
    auto family = family_creators.at(familyName)();
    apriltag_detector_add_family(C_DETECTOR,C_FAMILY(family));
}