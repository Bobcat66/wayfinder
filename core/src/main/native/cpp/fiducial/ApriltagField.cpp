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


#include "wfcore/fiducial/ApriltagField.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <format>
#include <stdexcept>

using json = nlohmann::json;

namespace impl {

    using namespace wf;

    static const JSONValidationFunctor* getTranslationValidator() {
        static JSONStructValidator validator(
            {
                {"x",getPrimitiveValidator<double>()},
                {"y",getPrimitiveValidator<double>()},
                {"z",getPrimitiveValidator<double>()}
            },
            {"x","y","z"}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    static const JSONValidationFunctor* getQuaternionValidator() {
        static JSONStructValidator validator(
            {
                {"W",getPrimitiveValidator<double>()},
                {"X",getPrimitiveValidator<double>()},
                {"Y",getPrimitiveValidator<double>()},
                {"Z",getPrimitiveValidator<double>()}
            },
            {"W","X","Y","Z"}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    static const JSONValidationFunctor* getRotationValidator() {
        static JSONStructValidator validator(
            {
                {"quaternion",getQuaternionValidator()}
            },
            {"quaternion"}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    static const JSONValidationFunctor* getPoseValidator() {
        static JSONStructValidator validator(
            {
                {"rotation", getRotationValidator()},
                {"translation", getTranslationValidator()}
            },
            {"rotation","translation"}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    static const JSONValidationFunctor* getApriltagValidator() {
        static JSONStructValidator validator(
            {
                {"ID", getPrimitiveValidator<int>()},
                {"pose", getPoseValidator()}
            },
            {"ID","pose"}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    static const JSONValidationFunctor* getTagsValidator() {
        static JSONArrayValidator validator(getApriltagValidator());
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    static const JSONValidationFunctor* getFieldValidator() {
        static JSONStructValidator validator(
            {
                {"length", getPrimitiveValidator<double>()},
                {"width", getPrimitiveValidator<double>()}
            },
            {"length","width"}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }
    
}

namespace wf {

    using enum WFStatus;

    const Apriltag* ApriltagField::getTag(int id) const noexcept {
        auto it = aprilTags.find(id);
        if (it != aprilTags.end()) {
            return &it->second;
        }
        return nullptr; // Not found
    }

    const JSONValidationFunctor* ApriltagField::getValidator_impl() {
        static JSONStructValidator validator(
            {
                {"tags", impl::getTagsValidator()},
                {"field", impl::getFieldValidator()}
            },
            {"tags","field"}
        );
        return static_cast<JSONValidationFunctor*>(&validator);
    }

    WFResult<JSON> ApriltagField::toJSON_impl(const ApriltagField& object) {
        JSON jobject = JSON::object();
        jobject["tags"] = JSON::array();
        for (const auto& [tagID, tag] : object.aprilTags) {
            auto q = tag.pose.rotation().toQuaternion();
            try {
                jobject["tags"].emplace_back(
                    JSON{
                        {"ID", tag.id},
                        {"pose", {
                            {"translation", {
                                {"x", tag.pose.x()},
                                {"y", tag.pose.y()},
                                {"z", tag.pose.z()}
                            }},
                            {"rotation", 
                                {"quaternion",{
                                    {"W",q.w()},
                                    {"X",q.x()},
                                    {"Y",q.y()},
                                    {"Z",q.z()}
                                }}
                            }
                        }}
                    }
                );
            } catch (const JSON::exception& e) {
                return WFResult<JSON>::failure(JSON_UNKNOWN,"JSON Error {} while serializing tag {}: {} {}",e.id,tagID,e.what());
            }
        }
        try {
            jobject["field"] = JSON{
                {"length",object.length},
                {"width",object.width}
            };
        } catch (const JSON::exception& e) {
            return WFResult<JSON>::failure(JSON_UNKNOWN,"JSON Error {} while serializing field constants: {}",e.id,e.what());
        }
        return WFResult<JSON>::success(std::move(jobject));
    }

    WFResult<ApriltagField> ApriltagField::fromJSON_impl(const JSON& jobject) {

        auto valid = (*getValidator())(jobject);
        if (!valid) return WFResult<ApriltagField>::propagateFail(valid);

        auto vres0 = validateProperties(jobject,{"tags","field"},"apriltag_field");
        if (!vres0) {
            return WFResult<ApriltagField>::propagateFail(vres0);
        }

        std::unordered_map<int,Apriltag> tags;
        for (const auto& tag_jobject : jobject["tags"]) {
            gtsam::Pose3 pose = {
                {
                    tag_jobject["pose"]["rotation"]["quaternion"]["W"].get<double>(),
                    tag_jobject["pose"]["rotation"]["quaternion"]["X"].get<double>(),
                    tag_jobject["pose"]["rotation"]["quaternion"]["Y"].get<double>(),
                    tag_jobject["pose"]["rotation"]["quaternion"]["Z"].get<double>()
                },
                {
                    tag_jobject["pose"]["translation"]["x"].get<double>(),
                    tag_jobject["pose"]["translation"]["y"].get<double>(),
                    tag_jobject["pose"]["translation"]["z"].get<double>()
                }
            };
            int id = tag_jobject["ID"].get<int>();
            Apriltag tag(
                id,
                std::move(pose)
            );
            tags.insert({id,std::move(tag)});
        }

        return WFResult<ApriltagField>::success(
            std::in_place,
            std::move(tags),
            jobject["field"]["length"].get<double>(),
            jobject["field"]["width"].get<double>()
        );
    }
}