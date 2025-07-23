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

    static wf::Apriltag makeApriltagFromJSON(const nlohmann::json& tagJSON) {
        return wf::Apriltag(
            tagJSON["ID"].get<int>(),
            gtsam::Pose3(
                gtsam::Rot3(
                    tagJSON["rotation"]["quaternion"]["W"].get<double>(),
                    tagJSON["rotation"]["quaternion"]["X"].get<double>(),
                    tagJSON["rotation"]["quaternion"]["Y"].get<double>(),
                    tagJSON["rotation"]["quaternion"]["Z"].get<double>()
                ),
                gtsam::Point3(
                    tagJSON["translation"]["x"].get<double>(),
                    tagJSON["translation"]["y"].get<double>(),
                    tagJSON["translation"]["z"].get<double>()
                )
            )
        );
    }

    static wf::ApriltagField makeFieldFromJSON(const json& j) {
        // TODO: add json schema validation
        std::unordered_map<int,wf::Apriltag> apriltags;
        for (json tagJSON : j["tags"]) {
            apriltags.insert({
                tagJSON["ID"].get<int>(),
                makeApriltagFromJSON(tagJSON)
            });
        }
        double width = j["field"]["width"].get<double>();
        double length = j["field"]["length"].get<double>();

        return wf::ApriltagField(apriltags,length,width);
        
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

    ApriltagField ApriltagField::loadFromJSONFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            // TODO: Make this a logger
            throw std::runtime_error("Failed to open file");
        }

        json jsonData;
        try {
            file >> jsonData;
        } catch (const json::parse_error& e) {
            // TODO: Logger
            throw std::runtime_error(std::format("Parse error: {}", e.what()));
        }
        return impl::makeFieldFromJSON(jsonData);
    }
    ApriltagField ApriltagField::loadFromJSONString(const std::string& jsonstr) {
        json jsonData;
        try {
            jsonData = json::parse(jsonstr);
        } catch (const json::parse_error& e) {
            // TODO: Logger
            throw std::runtime_error(std::format("Parse error: {}", e.what()));
        }

        return impl::makeFieldFromJSON(jsonData);
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
        auto vres0 = validateProperties(jobject,{"tags","field"},"apriltag_field");
        if (!vres0) {
            return WFResult<ApriltagField>::propagateFail(vres0);
        }

        std::unordered_map<int,Apriltag> tags;
        if (!jobject["tags"].is_array()) {
            return WFResult<ApriltagField>::failure(
                JSON_INVALID_TYPE,
                "apriltag_field.tags is not an array"
            );
        }
        for (const auto& tag_jobject : jobject["tags"]) {
            if (!validateProperties(tag_jobject,{"ID","pose"},"apriltag"))
                return WFResult<ApriltagField>::failure(JSON_PROPERTY_NOT_FOUND);
            
            int id;
            if (auto idres = getProperty<int>(tag_jobject,"ID","apriltag")) { id = idres.value(); } 
            else { return WFResult<ApriltagField>::propagateFail(idres); }

            auto vres1 = validateProperties(tag_jobject["pose"],{"rotation","translation"},"apriltag.pose");
            if (!vres1) return WFResult<ApriltagField>::propagateFail(vres1);

            auto vres2 = validateProperties(tag_jobject["pose"]["translation"],{"x","y","z"},"apriltag.pose.translation");
            if (!vres2) return WFResult<ApriltagField>::propagateFail(vres2);

            double x;
            double y;
            double z;
            if (auto xres = getProperty<double>(tag_jobject["pose"]["translation"],"x")) { x = xres.value(); } 
            else { return WFResult<ApriltagField>::propagateFail(xres); }

            if (auto yres = getProperty<double>(tag_jobject["pose"]["translation"],"y")) { y = yres.value(); }
            else { return WFResult<ApriltagField>::propagateFail(yres); }

            if (auto zres = getProperty<double>(tag_jobject["pose"]["translation"],"z")) { x = zres.value(); }
            else { return WFResult<ApriltagField>::failure(zres.status()); }



            if (!tag_jobject["pose"]["rotation"].contains("quaternion")) {
                return WFResult<ApriltagField>::failure(
                    JSON_PROPERTY_NOT_FOUND,
                    "apriltag.pose.rotation does not contain property quaternion"
                );
            }

            double wq;
            double xq;
            double yq;
            double zq;
            if (auto wqres = getProperty<double>(tag_jobject["pose"]["translation"]["quaternion"],"W")) { wq = wqres.value(); }
            else { return WFResult<ApriltagField>::failure(wqres.status()); }

            if (auto xqres = getProperty<double>(tag_jobject["pose"]["translation"]["quaternion"],"X")) { xq = xqres.value(); }
            else { return WFResult<ApriltagField>::failure(xqres.status()); }

            if (auto yqres = getProperty<double>(tag_jobject["pose"]["translation"]["quaternion"],"Y")) { yq = yqres.value();} 
            else { return WFResult<ApriltagField>::failure(yqres.status()); }

            if (auto zqres = getProperty<double>(tag_jobject["pose"]["translation"]["quaternion"],"Z")) { zq = zqres.value(); }
            else { return WFResult<ApriltagField>::failure(zqres.status()); }

            gtsam::Pose3 pose(
                {wq,xq,yq,zq},
                {x,y,z}
            );
            //tags[id] = {id,std::move(pose)};
        }

        return WFResult<ApriltagField>::failure(UNKNOWN); // placeholder
    }
}