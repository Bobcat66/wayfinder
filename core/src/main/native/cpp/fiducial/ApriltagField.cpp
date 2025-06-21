/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of Wayfinder.
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

    static wf::ApriltagField getFieldFromJson(const json& j) {
        std::unordered_map<int,wf::Apriltag> apriltags;
        for (json tagJSON : j["tags"]) {
            apriltags.insert({
                tagJSON["ID"].get<int>(),
                makeApriltagFromJSON(tagJSON)
            });
        }
        double width = j["field"]["width"].get<double>();
        double length = j["field"]["length"].get<double>();

        return wf::ApriltagField()
        
    }
}

namespace wf {
    const Apriltag* ApriltagField::getTag(int id) const noexcept {
        auto it = aprilTags.find(id);
        if (it != aprilTags.end()) {
            return &it->second;
        }
        return nullptr; // Not found
    }

    ApriltagField ApriltagField::loadFromJSONFile(const std::string& filepath) {

    }
    ApriltagField ApriltagField::loadFromJSONString(const std::string& json) {

    }
}