#pragma once

#include <vector>
#include <string>

#include <opencv2/core.hpp>
#include "wfcore/video/video_types.h"

namespace wf {
    struct ObjectDetection {
        int objectClass;
        double confidence;
        double percentArea;
        std::vector<cv::Point2d> cornerPixels;
        std::vector<cv::Point2d> cornerAngles;
        ObjectDetection(
            int objectClass_, double confidence_, double percentArea_,
            std::vector<cv::Point2d> cornerPixels_,
            std::vector<cv::Point2d> cornerAngles_
        ) : objectClass(objectClass_), confidence(confidence_), percentArea(percentArea_),
            cornerPixels(std::move(cornerPixels_)), cornerAngles(std::move(cornerAngles_)) {}
    };

    class InferenceEngine {
        public:
        virtual ~InferenceEngine() = default;
        virtual bool loadModel(const std::string& modelPath) = 0;
        [[nodiscard]] 
        virtual std::vector<ObjectDetection> infer(const Frame& input) noexcept = 0;
        virtual std::string modelFormat() const = 0; // the model file extension expected by this inference engine
    };
}