// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtsam/geometry/Pose3.h>

#include <string>
#include <memory>
#include <unordered_map>

namespace wf {

    struct Apriltag {
        int id;
        gtsam::Pose3 pose;

        std::string dumpJSON() const;
    };

    // Apriltag Layout for an FRC Field
    struct ApriltagField {
        std::unordered_map<int, Apriltag> aprilTags;
        std::string tagFamily;
        double tagSize;
        ApriltagField(std::unordered_map<int, Apriltag> aprilTags_, std::string tagFamily_, double tagSize_)
        : aprilTags(std::move(aprilTags_)), tagFamily(std::move(tagFamily_)), tagSize(tagSize_) {}
        const Apriltag* getTag(int id) const noexcept;

        // dumps JSON string
        std::string dumpJSON() const;

        static ApriltagField loadFromJSONFile(const std::string& filepath);
        static ApriltagField loadFromJSONString(const std::string& json);
    };

}