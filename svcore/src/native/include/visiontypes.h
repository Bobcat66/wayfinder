#pragma once

#include <opencv2/core/mat.hpp>

typedef struct {
    int id;
    cv::Mat corners;
    double decisionMargin;
    double hammingDistance;
} FiducialObservation;

typedef struct {
    int id;
    cv::Mat corners;
    double decisionMargin;
    double hammingDistance;
    cv::Mat tagpose0;
    double error0;
    cv::Mat tagpose1;
    double error1;
} FiducialPoseObservation;




