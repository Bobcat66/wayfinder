#pragma once

#include "wfcore/fiducial/ApriltagField.h"
#include <string>

namespace wf {
    struct ApriltagConfiguration {
        ApriltagField map;
        std::string tagFamily;
        double tagSize;
    };
}