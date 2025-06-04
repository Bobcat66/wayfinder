// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "types.h"

#include <string>
#include <map>
#include <memory>

namespace wf {

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