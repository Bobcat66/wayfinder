// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#include "pipeline/ApriltagDetector.h"

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

using namespace wf;

typedef apriltag_family_t* (*apriltag_family_creator)();
typedef void (*apriltag_family_destructor)(apriltag_family_t*);

static constexpr apriltag_family_creator family_creators[] = {
    tag36h11_create,
    tag36h10_create,
    tag25h9_create,
    tag16h5_create,
    tagCircle21h7_create,
    tagCircle49h12_create,
    tagCustom48h12_create,
    tagStandard41h12_create,
    tagStandard52h13_create
};

static constexpr apriltag_family_destructor family_destructors[] = {
    tag36h11_destroy,
    tag36h10_destroy,
    tag25h9_destroy,
    tag16h5_destroy,
    tagCircle21h7_destroy,
    tagCircle49h12_destroy,
    tagCustom48h12_destroy,
    tagStandard41h12_destroy,
    tagStandard52h13_destroy
};

ApriltagDetector::ApriltagDetector() {
    cdetector = apriltag_detector_create();
}

ApriltagDetector::~ApriltagDetector() {
    // Destroy apriltag detector
    apriltag_detector_destroy(
        static_cast<apriltag_detector_t*>(cdetector)
    );

    // Destroy apriltag families
    for (auto& [familyID,family] : apriltagFamilies){
        family_destructors[familyID](
            static_cast<apriltag_family_t*>(family)
        );
    }
}

void ApriltagDetector::addFamily(TagFamily familyID){
    auto family = family_creators[familyID]();
    apriltag_detector_add_family(
        static_cast<apriltag_detector_t*>(cdetector),
        static_cast<apriltag_family_t*>(family)
    );
}
