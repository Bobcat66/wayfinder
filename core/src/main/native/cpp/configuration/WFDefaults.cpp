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

#include "wfcore/configuration/WFDefaults.h"
#include "wfcore/common/json_utils.h"
#include "jval/WFDefaults.jval.hpp"
#include "wfcore/common/jval_compat.h"
#include "wfcore/common/wfdef.h"

#include <shared_mutex>

namespace impl {
    using namespace wf;

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

    template<typename T>
    cv::Scalar toScalar(const std::vector<T>& vec) {
        return cv::Scalar(
            vec.size() > 0 ? vec[0] : 0,
            vec.size() > 1 ? vec[1] : 0,
            vec.size() > 2 ? vec[2] : 0,
            vec.size() > 3 ? vec[3] : 0
        );
    }


    static std::shared_mutex mtx_;
};


namespace wf {
    WFStatusResult WFDefaults::load(const JSON& jobject) {
        std::unique_lock lock(impl::mtx_);
        return getInstance().load_impl(jobject);
    }

    std::string WFDefaults::getTagField() {
        std::shared_lock lock(impl::mtx_);
        return getInstance().tagField_;
    }

    std::string WFDefaults::getTagFamily() {
        std::shared_lock lock(impl::mtx_);
        return getInstance().tagFamily_;
    }

    double WFDefaults::getTagSize() {
        std::shared_lock lock(impl::mtx_);
        return getInstance().tagSize_;
    }

    std::string WFDefaults::getModelFile() {
        std::shared_lock lock(impl::mtx_);
        return getInstance().modelFile_;
    }

    ApriltagDetectorConfig WFDefaults::getTagDetectorConfig() {
        std::shared_lock lock(impl::mtx_);
        return getInstance().tagDetectorConfig_;
    }

    QuadThresholdParams WFDefaults::getQtps() {
        std::shared_lock lock(impl::mtx_);
        return getInstance().qtps_;
    }

    TensorParameters WFDefaults::getTensorParameters() {
        std::shared_lock lock(impl::mtx_);
        return getInstance().tensorParameters_;
    }

    float WFDefaults::getNmsThreshold() {
        std::shared_lock lock(impl::mtx_);
        return getInstance().nmsThreshold_;
    }

    float WFDefaults::getConfThreshold() {
        std::shared_lock lock(impl::mtx_);
        return getInstance().confThreshold_;
    }

    ImageEncoding WFDefaults::getModelColorSpace() {
        std::shared_lock lock(impl::mtx_);
        return getInstance().modelColorSpace_;
    }

    WFStatusResult WFDefaults::load_impl(const JSON& jobject) {
        auto valres = (*jval::get_WFDefaults_validator())(jobject);
        if (!valres) return JVResToWF(valres);
        tagField_ = jobject["tagField"].get<std::string>();
        tagFamily_ = jobject["tagFamily"].get<std::string>();
        tagSize_ = jobject["tagSize"].get<double>();
        modelFile_ = jobject["modelFile"].get<std::string>();
        auto tdc_jobject = jobject["tagDetectorConfig"];
        tagDetectorConfig_ = {
            tdc_jobject["numThreads"].get<int>(),
            tdc_jobject["quadDecimate"].get<float>(),
            tdc_jobject["quadSigma"].get<float>(),
            tdc_jobject["refineEdges"].get<bool>(),
            tdc_jobject["decodeSharpening"].get<double>(),
            tdc_jobject["debug"].get<bool>()
        };
        auto qtps_jobject = jobject["qtps"];
        qtps_ = {
            qtps_jobject["minClusterPixels"].get<int>(),
            qtps_jobject["maxNumMaxima"].get<int>(),
            qtps_jobject["criticalAngleRads"].get<float>(),
            qtps_jobject["maxLineFitMSE"].get<float>(),
            qtps_jobject["minWhiteBlackDiff"].get<int>(),
            qtps_jobject["deglitch"].get<bool>()
        };
        auto tp_jobject = jobject["tensorParams"];
        tensorParameters_ = {
            tp_jobject["interleaved"].get<bool>(),
            tp_jobject["height"].get<int>(),
            tp_jobject["width"].get<int>(),
            tp_jobject["channels"].get<int>(),
            tp_jobject["scale"].get<float>(),
            impl::toScalar(tp_jobject["stds"].get<std::vector<double>>()),
            impl::toScalar(tp_jobject["means"].get<std::vector<double>>())
        };
        engineType_ = impl::parseInferenceEngineType(jobject["engineType"].get<std::string>());
        modelArch_ = impl::parseModelArch(jobject["modelArch"].get<std::string>());
        modelColorSpace_ = impl::parseEncoding(jobject["modelColorSpace"].get<std::string>());
        nmsThreshold_ = jobject["nmsThreshold"].get<float>();
        confThreshold_ = jobject["confidenceThreshold"].get<float>();
        return WFStatusResult::success();
    }

    WFDefaults::WFDefaults() {
        std::unique_lock lock(impl::mtx_);
        // These are the fallback defaults if a JSON defaults file isn't found or if the one that is found is invalid
        // Also helps with unit testing
        tagField_ = "k2025ReefscapeWelded";
        tagFamily_ = "tag36h11";
        tagSize_ = 6.5;
        modelFile_ = "default.onnx";
        tagDetectorConfig_ = {
            .numThreads = 1,
            .quadDecimate = 2.0f,
            .quadSigma = 0.0f,
            .refineEdges = true,
            .decodeSharpening = 0.25,
            .debug = false
        };
        qtps_ = {
            .minClusterPixels = 5,
            .maxNumMaxima = 10,
            .criticalAngleRads = 0.0f,
            .maxLineFitMSE = 10.0f,
            .minWhiteBlackDiff = 5,
            .deglitch = false
        };
        engineType_ = InferenceEngineType::CV_CPU;
        modelArch_ = ModelArch::YOLO;
        modelColorSpace_ = ImageEncoding::RGB24;
        nmsThreshold_ = 0.5f;
        confThreshold_ = 0.5f;
    }
}