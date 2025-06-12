#pragma once

#include "wfcore/fiducial/ApriltagDetector.h"
#include "wfcore/pipeline/Pipeline.h"

namespace wf {

    struct ApriltagPipelineConfiguration {
        ApriltagDetectorConfig detConfig;
        QuadThresholdParams detQTPs;
        std::vector<int> detectorExcludes;
        std::vector<int> SolvePNPExcludes; // Does not effect tag relative solvePNP
        bool solveTagRelative; // Whether or not to solve tag relative
    };

    struct ObjectDetectPipelineConfiguration {
        std::string model;
        double confidence_threshold;
    };

    struct CameraIntrinsics {
        cv::Mat cameraMatrix;
        cv::Mat distCoeffs;
    };

    struct CameraConfiguration {
        int id;
        CameraIntrinsics intrinsics;
        std::string raw_format;
        
    };

    struct PipelineConfiguration {
        std::string name;
        PipelineType type;
        
    };
}