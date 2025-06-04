// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <gtsam/geometry/Pose3.h>
#include <string>
#include <map>
#include <memory>

namespace wf {

typedef struct {
    int id;
    gtsam::Pose3 pose;
} AprilTag;

typedef struct {
    std::map<int, AprilTag> aprilTags;
    std::string tagFamily;
    double tagSize;
} FieldLayout;

class FieldLoader {
    public:
    void loadFieldLayoutJSON(const std::string& filename);
    void setFieldLayout(FieldLayout& layout);
    std::string dumpFieldLayoutJSON() const;
    FieldLayout& getFieldLayout() const;
    FieldLayout getFieldLayoutCopy() const;
    const FieldLayout& getConstFieldLayout() const;
    private:
    FieldLayout fieldLayout;
};

}