#include "wfdetail/validation/inference_validators.h"

namespace wf::detail {
    const JSONValidationFunctor* getModelArchValidator() {
        static JSONEnumValidator validator({
            "YOLO",
            "SSD",
            "RETINA_NET",
            "RCNN"
        });
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    const JSONValidationFunctor* getInferenceEngineTypeValidator() {
        static JSONEnumValidator validator({
            "CV_CPU",
            "CV_OPENCL",
            "CV_VULKAN",
            "CUDA",
            "OpenVINO",
            "RKNN",
            "CoreML",
            "ROCm",
            "EdgeTPU",
            "HailoRT"
        });
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    const JSONValidationFunctor* getMeansAndSTDsValidator() {
        static JSONArrayValidator validator(getPrimitiveValidator<double>(),1,4);
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    const JSONValidationFunctor* getTensorParamsValidator() {
        static JSONStructValidator validator(
            {
                {"interleaved", getPrimitiveValidator<bool>()},
                {"height", getPrimitiveValidator<int>()},
                {"width", getPrimitiveValidator<int>()},
                {"channels", getPrimitiveValidator<int>()},
                {"scale", getPrimitiveValidator<double>()},
                {"stds", getMeansAndSTDsValidator()},
                {"means", getMeansAndSTDsValidator()}
            },
            {"interleaved","height","width","channels","scale","stds","means"},
            {}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    const JSONValidationFunctor* getFilterParamsValidator() {
        static JSONStructValidator validator(
            {
                {"nmsThreshold", getPrimitiveValidator<double>()},
                {"confidenceThreshold", getPrimitiveValidator<double>()}
            },
            {"nmsThreshold","confidenceThreshold"},
            {}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }
}