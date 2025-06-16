#pragma once

#include "wfcore/inference/InferenceEngine.h"

#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>

// A simple reference implementation of a CPU inference engine, using OpenCV's DNN module.
namespace wf {
    class CPUInferenceEngine : public InferenceEngine {
    public:
        CPUInferenceEngine();
        std::string modelFormat() const override {return "onnx";}
        bool loadModel(const std::string& modelPath) override;
        [[nodiscard]] 
        std::vector<ObjectDetection> infer(const Frame& input) noexcept override;
    private:

    }
}