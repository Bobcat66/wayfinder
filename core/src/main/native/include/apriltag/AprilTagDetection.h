#pragma once

#include <opencv2/core/types.hpp>

#include <vector>
#include <string>

namespace wf {

    struct AprilTagDetection {
        int id;
        std::vector<cv::Point2d> corners;
        double decisionMargin;
        double hammingDistance;
        std::string family;
        AprilTagDetection(
            int id_, 
            std::vector<cv::Point2d> corners_, 
            double decisionMargin_, double hammingDistance_,
            std::string family_
        ) : id(id_), 
            corners(std::move(corners_)),
            decisionMargin(decisionMargin_), hammingDistance(hammingDistance_),
            family(std::move(family_)) {}
    };
    
}