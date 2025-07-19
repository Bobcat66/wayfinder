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

#include "wfcore/common/status.h"
#include "wfcore/hardware/CameraConfiguration.h"
#include "wfcore/processes/VisionWorkerConfig.h"
#include "wfcore/fiducial/ApriltagField.h"
#include <filesystem>
#include <optional>
#include <vector>

namespace wf {

    enum class ConfigLoaderStatus {
        Ok,
        FileNotOpened,
        SchemaViolation,
        JSONParseError
    };

    class ConfigLoader : public LoggedStatusfulObject<ConfigLoaderStatus,ConfigLoaderStatus::Ok> {
    public:
        ConfigLoader(std::filesystem::path localDirPath, std::filesystem::path resourceDirPath);
        std::optional<CameraConfiguration> loadCameraConfig(const std::string& filename) const noexcept;
        std::optional<VisionWorkerConfig> loadWorkerConfig(const std::string& filename) const noexcept;
        bool storeVisionWorkerConfig(const std::string& filename,const VisionWorkerConfig& config) const noexcept;
        bool storeCameraConfig(const std::string& filename, const CameraConfiguration& config) const noexcept;
        std::vector<std::string> searchForCameraConfigFiles() const noexcept;
        std::vector<std::string> searchForVisionWorkerConfigFiles() const noexcept;
        std::optional<ApriltagField> loadField(const std::string& filename) const noexcept;
    private:
        // Path to the local/ directory containing local configuration files
        const std::filesystem::path localDirPath_;

        // Path to the resources/ directory containing WIPS resources (apriltag fields, models, etc.)
        const std::filesystem::path resourceDirPath_;
    };
}