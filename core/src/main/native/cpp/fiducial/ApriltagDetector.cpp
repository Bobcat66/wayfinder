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

#include "wfcore/fiducial/ApriltagDetector.h"
#include "wfcore/logging/LoggerManager.h"

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

namespace wf {

    static loggerPtr logger = LoggerManager::getInstance().getLogger("ApriltagDetector",LogGroup::General);

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
        for (auto& [familyName, family] : families){
            family_destructors.at(familyName)(C_FAMILY(family));
        }
    }

    std::vector<ApriltagDetection> ApriltagDetector::detect(int height, int width, int stride, uint8_t* buf) const noexcept {
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

    void ApriltagDetector::setQuadThresholdParams(const QuadThresholdParams params) {
        auto& qtp = C_DETECTOR->qtp;
        qtp.min_cluster_pixels = params.minClusterPixels;
        qtp.max_nmaxima = params.maxNumMaxima;
        qtp.critical_rad = params.criticalAngleRads;
        qtp.cos_critical_rad = std::cos(params.criticalAngleRads);
        qtp.max_line_fit_mse = params.maxLineFitMSE;
        qtp.min_white_black_diff = params.minWhiteBlackDiff;
        qtp.deglitch = params.deglitch;
    }

    void ApriltagDetector::setConfig(const ApriltagDetectorConfig config) {
        C_DETECTOR->nthreads = config.numThreads;
        C_DETECTOR->quad_decimate = config.quadDecimate;
        C_DETECTOR->quad_sigma = config.quadSigma;
        C_DETECTOR->refine_edges = config.refineEdges;
        C_DETECTOR->decode_sharpening = config.decodeSharpening;
        C_DETECTOR->debug = config.debug;
    }

    void ApriltagDetector::removeFamily(const std::string& familyName) {
        auto it = families.find(familyName);
        if (it == families.end()) {
            return;
        }
        apriltag_detector_remove_family(
            C_DETECTOR,
            C_FAMILY(it->second)
        );
        family_destructors.at(familyName)(C_FAMILY(it->second));
        families.erase(it);
    }

    void ApriltagDetector::addFamily(const std::string& familyName){
        if (families.find(familyName) != families.end()){
            return;
        }
        auto it = family_creators.find(familyName);
        if (it == family_creators.end()) {
            logger->warn("{} is not a valid apriltag family",familyName);
            return;
        }
        auto family = (it->second)();
        families[familyName] = family;
        apriltag_detector_add_family(C_DETECTOR,C_FAMILY(family));
    }

    void ApriltagDetector::clearFamilies() {
        for (auto it = families.begin(); it != families.end();) {
            apriltag_detector_remove_family(
                C_DETECTOR,
                C_FAMILY(it->second)
            );
            family_destructors.at(it->first)(C_FAMILY(it->second));
            it = families.erase(it);
        }
    }

}