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

#pragma once

#include "wfcore/fiducial/ApriltagField.h"
#include "wfcore/configuration/ResourceManager.h"
#include "wfcore/common/status.h"

// This handles apriltag fields. TODO: Connect the handlers to a global apriltag field, rather than having each handler maintain its own field
namespace wf {
    class ApriltagFieldHandler {
    public:
        ApriltagFieldHandler(const ResourceManager& resourceManager_)
        : resourceManager(resourceManager_) {}
        WFStatusResult loadField(std::string_view newFieldName);
        const ApriltagField& getField() const noexcept { return field; }
        const std::string& getFieldName() const noexcept { return fieldName; }
    private:
        std::string fieldName;
        ApriltagField field;
        const ResourceManager& resourceManager;
    };
}