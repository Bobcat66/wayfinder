// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <gtsam/geometry/Pose3.h>

#include <string>
#include <memory>
#include <unordered_map>

namespace wf {

    class AprilTag {
        public:
        int id;
        gtsam::Pose3 pose;

        std::string dumpJSON() const;
    };

    // AprilTag Layout for an FRC Field
    class AprilTagField {
        public:
        std::unordered_map<int, AprilTag> aprilTags;
        std::string tagFamily;
        double tagSize;
        const AprilTag* getTag(int id) const;/* {
            auto it = aprilTags.find(id);
            if (it != aprilTags.end()) {
                return &it->second;
            }
            return nullptr; // Not found
        }*/

        // dumps JSON string
        std::string dumpJSON() const;

        static AprilTagField loadFromJSONFile(const std::string& filepath);
        static AprilTagField loadFromJSONString(const std::string& json);
    };

}