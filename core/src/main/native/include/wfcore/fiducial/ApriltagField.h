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


#pragma once

#include <gtsam/geometry/Pose3.h>

#include <string>
#include <memory>
#include <unordered_map>

namespace wf {

    struct Apriltag {
        int id;
        gtsam::Pose3 pose;
        Apriltag(int id_, gtsam::Pose3 pose_) : id(id_), pose(std::move(pose)) {}
        std::string dumpJSON() const;
    };

    // Apriltag Layout for an FRC Field
    struct ApriltagField {
        std::unordered_map<int, Apriltag> aprilTags;
        double length;
        double width;
        ApriltagField(std::unordered_map<int, Apriltag> aprilTags_, double length_, double width_)
        : aprilTags(std::move(aprilTags_)), length(length_), width(width_) {}
        const Apriltag* getTag(int id) const noexcept;

        // dumps JSON string
        std::string dumpJSON() const;

        static ApriltagField loadFromJSONFile(const std::string& filepath);
        static ApriltagField loadFromJSONString(const std::string& jsonstr);
    };

}