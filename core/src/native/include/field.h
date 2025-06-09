// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "types.h"

#include <string>
#include <memory>

namespace wf {

    struct FieldLayout {
        std::map<int, AprilTag> aprilTags;
        std::string tagFamily;
        double tagSize;
        const AprilTag* getTag(int id) const {
            auto it = aprilTags.find(id);
            if (it != aprilTags.end()) {
                return &it->second;
            }
            return nullptr; // Not found
        }
    };

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