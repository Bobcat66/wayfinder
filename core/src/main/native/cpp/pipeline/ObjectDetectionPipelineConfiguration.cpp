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
#include "wfcore/common/envutils.h"
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
            {"interleaved","height","width","channels","scale","stds","means"}
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

    template<typename T>
    cv::Scalar toScalar(const std::vector<T>& vec) {
        return cv::Scalar(
            vec.size() > 0 ? vec[0] : 0,
            vec.size() > 1 ? vec[1] : 0,
            vec.size() > 2 ? vec[2] : 0,
            vec.size() > 3 ? vec[3] : 0
        );
    }
};

namespace wf {
    const JSONValidationFunctor* ObjectDetectionPipelineConfiguration::getValidator_impl() {
        static JSONStructValidator validator(
            {
                {"modelFile", getPrimitiveValidator<std::string>()},
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

    WFResult<JSON> ObjectDetectionPipelineConfiguration::toJSON_impl(const ObjectDetectionPipelineConfiguration& object) {
        try {
            JSON jobject = {
                {"modelFile", object.modelFile},
                {"modelArch", impl::modelArchToString(object.modelArch)},
                {"engineType", impl::inferenceEngineTypeToString(object.engineType)},
                {"tensorParams", {
                    {"interleaved", object.tensorParams.interleaved},
                    {"height", object.tensorParams.height},
                    {"width", object.tensorParams.width},
                    {"channels", object.tensorParams.channels},
                    {"scale", object.tensorParams.scale},
                    {"stds", {
                        object.tensorParams.stds[0],
                        object.tensorParams.stds[1],
                        object.tensorParams.stds[2],
                        object.tensorParams.stds[3]
                    }},
                    {"means", {
                        object.tensorParams.means[0],
                        object.tensorParams.means[1],
                        object.tensorParams.means[2],
                        object.tensorParams.means[3]
                    }}
                }},
                {"modelColorSpace", impl::encodingToString(object.modelColorSpace)},
                {"filterParams", {
                    {"nmsThreshold", object.filterParams.nmsThreshold},
                    {"confidenceThreshold", object.filterParams.confidenceThreshold}
                }}
            };
            return WFResult<JSON>::success(std::move(jobject));
        } catch (const JSON::exception& e) {
            return WFResult<JSON>::failure(WFStatus::JSON_UNKNOWN,e.what());
        }
    }

    // ts is lowkey scuffed, refactor after 1.0
    WFResult<ObjectDetectionPipelineConfiguration> ObjectDetectionPipelineConfiguration::fromJSON_impl(const JSON& jobject) {
        auto valid = (*getValidator())(jobject);
        if (!valid) return WFResult<ObjectDetectionPipelineConfiguration>::propagateFail(valid);

        auto stds = impl::toScalar(jobject["tensorParams"]["stds"].get<std::vector<double>>());
        auto means = impl::toScalar(jobject["tensorParams"]["means"].get<std::vector<double>>());

        std::string defaultFile;
        std::string defaultArch;
        std::string defaultEngine;
        bool defaultInterleaved;
        int defaultHeight;
        int defaultWidth;
        int defaultChannels;
        double defaultScale;
        std::vector<double> defaultStds;
        std::vector<double> defaultMeans;
        std::string defaultColorSpace;
        double defaultNMSThreshold;
        double defaultConfThreshold;
        

        if (auto pathopt = env::getVar("WF_DEFAULT_MODEL")) {
            defaultFile = std::move(pathopt.value());
        } else {
            if (!jobject.contains("modelFile"))
                return WFResult<ObjectDetectionPipelineConfiguration>::failure(
                    static_cast<WFStatus>(env::getError()),
                    "Object detection config does not specify a model file and failed to retrieve fallback from environment"
                );
        }

        if (auto archopt = env::getVar("WF_DEFAULT_MODEL_ARCH")){
            defaultArch = std::move(archopt.value());
        } else {
            if (!jobject.contains("modelArch"))
                return WFResult<ObjectDetectionPipelineConfiguration>::failure(
                    static_cast<WFStatus>(env::getError()),
                    "Object detection config does not specify a model architecture and failed to retrieve fallback from environment"
                );
        }

        if (auto ieopt = env::getVar("WF_DEFAULT_IE")) {
            defaultEngine = std::move(ieopt.value());
        } else {
            if (!jobject.contains("engineType"))
                return WFResult<ObjectDetectionPipelineConfiguration>::failure(
                    static_cast<WFStatus>(env::getError()),
                    "Object detection config does not specify an inference engine type and failed to retrieve fallback from environment"
                );
        }

        if (!jobject.contains("tensorParams")) {
            auto ilopt = env::getBool("WF_DEFAULT_MODEL_INTERLEAVED");
            auto widthopt = env::getInt("WF_DEFAULT_MODEL_TENSORWIDTH");
            auto heightopt = env::getInt("WF_DEFAULT_MODEL_TENSORHEIGHT");
            auto channelopt = env::getInt("WF_DEFAULT_MODEL_CHANNELS");
            auto scaleopt = env::getDouble("WF_DEFAULT_MODEL_TENSORSCALE");
            auto meansopt = env::getDoubleArr("WF_DEFAULT_MODEL_MEANS");
            auto stdsopt = env::getDoubleArr("WF_DEFAULT_MODEL_STDS");
            if (!(
                ilopt 
                && widthopt
                && heightopt
                && channelopt
                && scaleopt
                && meansopt
                && stdsopt
            )) {

                return WFResult<ObjectDetectionPipelineConfiguration>::failure(
                    static_cast<WFStatus>(env::getError()),
                    "Object detection config does not specify tensor parameters and failed to retrieve fallback from environment"
                );
            } else {
                defaultInterleaved = ilopt.value();
                defaultHeight = heightopt.value();
                defaultWidth = widthopt.value();
                defaultChannels = channelopt.value();
                defaultScale = scaleopt.value();
                defaultMeans = std::move(meansopt.value());
                defaultStds = std::move(stdsopt.value());
            }

        }

        if (auto csopt = env::getVar("WF_DEFAULT_MODEL_COLORSPACE")) {
            defaultColorSpace = std::move(csopt.value());
        } else {
            if (!jobject.contains("modelColorSpace"))
                return WFResult<ObjectDetectionPipelineConfiguration>::failure(
                    static_cast<WFStatus>(env::getError()),
                    "Object detection config does not specify a model color space and failed to retrieve fallback from environment"
                );
        }

        if (!jobject.contains("filterParams")) {
            auto nmsopt = env::getDouble("WF_DEFAULT_MODEL_NMSTHRESHOLD");
            auto confopt = env::getDouble("WF_DEFAULT_MODEL_CONFTHRESHOLD");
            if (!(nmsopt && confopt)) {
                return WFResult<ObjectDetectionPipelineConfiguration>::failure(
                    static_cast<WFStatus>(env::getError()),
                    "Object detection config does not specify filtering parameters and failed to retrieve fallback from environment"
                );
            } else {
                defaultNMSThreshold = nmsopt.value();
                defaultConfThreshold = confopt.value();
            }
        }
        bool tps = jobject.contains("tensorParams");
        TensorParameters params;
        if (tps) {
            JSON tpjobject = jobject["tensorParams"];
            params = TensorParameters{
                tpjobject["interleaved"].get<bool>(),
                tpjobject["height"].get<int>(),
                tpjobject["width"].get<int>(),
                tpjobject["channels"].get<int>(),
                tpjobject["scale"].get<float>(),
                impl::toScalar(tpjobject["stds"].get<std::vector<double>>()),
                impl::toScalar(tpjobject["means"].get<std::vector<double>>()),
            };
        } else {
            params = TensorParameters{
                defaultInterleaved,
                defaultHeight,
                defaultWidth,
                defaultChannels,
                static_cast<float>(defaultScale),
                impl::toScalar(defaultStds),
                impl::toScalar(defaultMeans)
            };
        }
        bool fps = jobject.contains("filterParams");
        IEFilteringParams fparams;
        if (fps) {
            JSON fpjobject = jobject["filterParams"];
            fparams = IEFilteringParams{
                fpjobject["nmsThreshold"],
                fpjobject["confidenceThreshold"]
            };
        } else {
            fparams = IEFilteringParams{
                static_cast<float>(defaultNMSThreshold),
                static_cast<float>(defaultConfThreshold)
            };
        }
        return WFResult<ObjectDetectionPipelineConfiguration>::success(
            std::in_place,
            getJSONOpt(jobject,"modelFile",std::move(defaultFile)),
            impl::parseModelArch(getJSONOpt(jobject,"modelArch",std::move(defaultArch))),
            impl::parseInferenceEngineType(getJSONOpt(jobject,"engineType",std::move(defaultEngine))),
            std::move(params),
            impl::parseEncoding(getJSONOpt(jobject,"modelColorSpace",std::move(defaultColorSpace))),
            std::move(fparams)
        );
    }
}