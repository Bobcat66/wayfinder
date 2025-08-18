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

#include "wfcore/processes/VisionWorkerConfig.h"
#include "jval/VisionWorkerConfig.jval.hpp"
#include "wfcore/common/json_utils.h"
#include "jval/ObjectDetectionPipelineConfig.jval.hpp"
#include "jval/ApriltagPipelineConfig.jval.hpp"

namespace impl {
    using namespace wf;
    static WFResult<std::variant<ApriltagPipelineConfiguration,ObjectDetectionPipelineConfiguration>> getPipelineConfig(const JSON& jobject) {
        static JSONVariantDecoder<ApriltagPipelineConfiguration,ObjectDetectionPipelineConfiguration> decoder(
            {
                jval::get_ApriltagPipelineConfig_validator(),
                [](const JSON& jobject) -> WFResult<ApriltagPipelineConfiguration> {
                    return ApriltagPipelineConfiguration::fromJSON(jobject);
                }
            },
            {
                jval::get_ObjectDetectionPipelineConfig_validator(),
                [](const JSON& jobject) -> WFResult<ObjectDetectionPipelineConfiguration> {
                    return ObjectDetectionPipelineConfiguration::fromJSON(jobject);
                }
            }
        );

        return decoder.decode(jobject);
    }
    static PipelineType decodePipelineType(const JSON& jobject) {
        auto str = jobject.get<std::string>();
        if (str == "Apriltag") return PipelineType::Apriltag;
        if (str == "ObjDetect") return PipelineType::ObjDetect;
        return PipelineType::NullType;
    }

    static std::string encodePipelineType(PipelineType ptype) {
        switch (ptype) {
            case PipelineType::Apriltag: return "Apriltag";
            case PipelineType::ObjDetect: return "ObjDetect";
            default: return "NullType";
        }
    }

    template <class... Lambdas>
    struct LambdaVisitor : Lambdas... {
        using Lambdas::operator()...;
    };

    // CTAD
    template<class... Lambdas>
    LambdaVisitor(Lambdas...) -> LambdaVisitor<Lambdas...>;
}

namespace wf {
    const jval::JSONValidationFunctor* VisionWorkerConfig::getValidator_impl() {
        return jval::get_VisionWorkerConfig_validator();
    }

    WFResult<VisionWorkerConfig> VisionWorkerConfig::fromJSON_impl(const JSON& jobject) {
        auto valres = validate(jobject);
        if (!valres)
            return WFResult<VisionWorkerConfig>::propagateFail(valres);

        auto pcfgRes = impl::getPipelineConfig(jobject["pipelineConfig"]);
        if (!pcfgRes)
            return WFResult<VisionWorkerConfig>::propagateFail(pcfgRes);
        
        return WFResult<VisionWorkerConfig>::success(
            std::in_place,
            jobject["camera_nickname"].get<std::string>(),
            jobject["name"].get<std::string>(),
            getJSONOpt(jobject,"inputFormat",StreamFormat()),
            getJSONOpt(jobject,"outputFormat",StreamFormat()),
            jobject["stream"].get<bool>(),
            getJSONOpt(jobject,"raw_port",0),
            getJSONOpt(jobject,"processed_port",0),
            impl::decodePipelineType(jobject["pipelineType"]),
            std::move(pcfgRes.value())
        );
    }
    WFResult<JSON> VisionWorkerConfig::toJSON_impl(const VisionWorkerConfig& config) {
        // placeholder
        JSON pcfg_jobject;
        WFStatus visitorStatus = WFStatus::OK;
        std::string visitorMsg;
        std::visit(impl::LambdaVisitor{
            [&pcfg_jobject,&visitorStatus,&visitorMsg](ApriltagPipelineConfiguration apcfg) -> void {
                auto res = ApriltagPipelineConfiguration::toJSON(apcfg);
                if (!res) {
                    // Propagate error to enclosing scope
                    visitorStatus = res.status();
                    visitorMsg = res.what();
                    return;
                }
                pcfg_jobject = std::move(res.value());
            },
            [&pcfg_jobject,&visitorStatus,&visitorMsg](ObjectDetectionPipelineConfiguration odpcfg) -> void {
                auto res = ObjectDetectionPipelineConfiguration::toJSON(odpcfg);
                if (!res) {
                    visitorStatus = res.status();
                    visitorMsg = res.what();
                    return;
                }
                pcfg_jobject = std::move(res.value());
            }
        }, config.pipelineConfig);
        if (visitorStatus != WFStatus::OK)
            return WFResult<JSON>::failure(visitorStatus,visitorMsg);

        auto iformat_res = StreamFormat::toJSON(config.inputFormat);
        if (!iformat_res)
            return WFResult<JSON>::propagateFail(iformat_res);
        
        auto iformat_jobject = std::move(iformat_res.value());

        auto oformat_res = StreamFormat::toJSON(config.outputFormat);
        if (!oformat_res)
            return WFResult<JSON>::propagateFail(oformat_res);
        
        auto oformat_jobject = std::move(oformat_res.value());
        
        try {
            JSON jobject = {
                {"camera_nickname",config.camera_nickname},
                {"name",config.name},
                {"inputFormat",std::move(iformat_jobject)},
                {"outputFormat",std::move(oformat_jobject)},
                {"stream",config.stream},
                {"raw_port",config.raw_port},
                {"processed_port",config.processed_port},
                {"pipelineType",impl::encodePipelineType(config.pipelineType)},
                {"pipelineConfig",std::move(pcfg_jobject)}
            };
            return jobject;
        } catch (const JSON::exception& e) {
            return WFResult<JSON>::failure(WFStatus::JSON_UNKNOWN,e.what());
        }
    }
}