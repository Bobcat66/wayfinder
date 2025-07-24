/*
 * SPDX-License-Identifier: GPL-3.0-or-later
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

#include "wfcore/pipeline/ObjectDetectionPipelineConfiguration.h"
#include "wfcore/common/wfdef.h"
#include "wfcore/common/logging.h"
#include <string_view>

namespace impl {
    using namespace wf;

    static const JSONValidationFunctor* getModelArchValidator() {
        static JSONEnumValidator validator({
            "YOLO",
            "SSD",
            "RETINA_NET",
            "RCNN"
        });
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    static const JSONValidationFunctor* getInferenceEngineTypeValidator() {
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

    inline constexpr std::string_view inferenceEngineTypeToString(InferenceEngineType engine) {
        switch (engine) {
            case InferenceEngineType::CV_CPU:     return "CV_CPU";
            case InferenceEngineType::CV_OPENCL:  return "CV_OPENCL";
            case InferenceEngineType::CV_VULKAN:  return "CV_VULKAN";
            case InferenceEngineType::CUDA:       return "CUDA";
            case InferenceEngineType::OpenVINO:   return "OpenVINO";
            case InferenceEngineType::RKNN:       return "RKNN";
            case InferenceEngineType::CoreML:     return "CoreML";
            case InferenceEngineType::ROCm:       return "ROCm";
            case InferenceEngineType::EdgeTPU:    return "EdgeTPU";
            case InferenceEngineType::HailoRT:    return "HailoRT";
        }
        WF_UNREACHABLE;
    }

    inline constexpr InferenceEngineType parseInferenceEngineType(std::string_view str) {
        if (str == "CV_CPU") return InferenceEngineType::CV_CPU;
        if (str == "CV_OPENCL") return InferenceEngineType::CV_OPENCL;
        if (str == "CV_VULKAN") return InferenceEngineType::CV_VULKAN;
        if (str == "CUDA") return InferenceEngineType::CUDA;
        if (str == "OpenVINO") return InferenceEngineType::OpenVINO;
        if (str == "RKNN") return InferenceEngineType::RKNN;
        if (str == "CoreML") return InferenceEngineType::CoreML;
        if (str == "ROCm") return InferenceEngineType::ROCm;
        if (str == "EdgeTPU") return InferenceEngineType::EdgeTPU;
        if (str == "HailoRT") return InferenceEngineType::HailoRT;

        WF_UNREACHABLE;
    }

    inline constexpr std::string_view modelArchToString(ModelArch arch) {
        switch (arch) {
            case ModelArch::YOLO:       return "YOLO";
            case ModelArch::SSD:        return "SSD";
            case ModelArch::RETINA_NET: return "RETINA_NET";
            case ModelArch::RCNN:       return "RCNN";
        }
        WF_UNREACHABLE;
    }

    inline constexpr ModelArch parseModelArch(std::string_view str) {
        if (str == "YOLO")        return ModelArch::YOLO;
        if (str == "SSD")         return ModelArch::SSD;
        if (str == "RETINA_NET")  return ModelArch::RETINA_NET;
        if (str == "RCNN")        return ModelArch::RCNN;

        WF_UNREACHABLE;
    }

    static const JSONValidationFunctor* getEncodingValidator() {
        static JSONEnumValidator validator({
            "BGR24",
            "RGB24",
            "RGB565",
            "Y8",
            "Y16",
            "YUYV",
            "UYVY",
            "RGBA",
            "BGRA",
            "MJPEG"
        });
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    inline constexpr ImageEncoding parseEncoding(const std::string& name) {
        if (name == "BGR24") return ImageEncoding::BGR24;
        if (name == "RGB24") return ImageEncoding::RGB24;
        if (name == "RGB565") return ImageEncoding::RGB565;
        if (name == "Y8") return ImageEncoding::Y8;
        if (name == "Y16") return ImageEncoding::Y16;
        if (name == "YUYV") return ImageEncoding::YUYV;
        if (name == "UYVY") return ImageEncoding::UYVY;
        if (name == "RGBA") return ImageEncoding::RGBA;
        if (name == "BGRA") return ImageEncoding::BGRA;
        if (name == "MJPEG") return ImageEncoding::MJPEG;
        return ImageEncoding::UNKNOWN;
    }

    inline constexpr std::string_view encodingToString(ImageEncoding encoding) {
        using enum ImageEncoding;
        switch (encoding) {
            case BGR24: return "BGR24";
            case RGB24: return "RGB24";
            case RGB565: return "RGB565";
            case Y8: return "Y8";
            case Y16: return "Y16";
            case YUYV: return "YUYV";
            case UYVY: return "UYVY";
            case RGBA: return "RGBA";
            case BGRA: return "BGRA";
            case MJPEG: return "MJPEG";
            default: return "UNKNOWN";
        }
    }   

    static const JSONValidationFunctor* getMeansAndSTDsValidator() {
        static JSONArrayValidator validator(getPrimitiveValidator<double>(),1,4);
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    static const JSONValidationFunctor* getTensorParamsValidator() {
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
            {}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    static const JSONValidationFunctor* getFilterParamsValidator() {
        static JSONStructValidator validator(
            {
                {"nmsThreshold", getPrimitiveValidator<double>()},
                {"confidenceThreshold", getPrimitiveValidator<double>()}
            },
            {"nmsThreshold","confidenceThreshold"}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }
};

namespace wf {
    const JSONValidationFunctor* ObjectDetectionPipelineConfiguration::getValidator_impl() {
        static JSONStructValidator validator(
            {
                {"modelPath", getPrimitiveValidator<std::string>()},
                {"modelArch", impl::getModelArchValidator()},
                {"engineType", impl::getInferenceEngineTypeValidator()},
                {"tensorParams", impl::getTensorParamsValidator()},
                {"modelColorSpace", impl::getEncodingValidator()},
                {"filterParams", impl::getFilterParamsValidator()}
            },
            {}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }
}