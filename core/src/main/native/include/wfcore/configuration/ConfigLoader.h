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

#include "wfcore/common/StatusfulObject.h"
#include "wfcore/hardware/CameraConfiguration.h"
#include "wfcore/processes/VisionWorkerConfig.h"
#include <filesystem>
#include <optional>

namespace wf {

    enum class ConfigLoaderStatus {
        Ok,
        FileNotFound,
        InvalidJSON
    };

    class ConfigLoader : public LoggedStatusfulObject<ConfigLoaderStatus,ConfigLoaderStatus::Ok> {
    public:
        std::optional<CameraConfiguration> loadCameraConfig(const std::filesystem::path& path);
        std::optional<VisionWorkerConfig> loadWorkerConfig(const std::filesystem::path& path);
        bool storeVisionWorkerConfig(const std::filesystem::path& path,const VisionWorkerConfig& newConfig);
        bool storeCameraConfig(const std::filesystem::path& path, const CameraConfiguration& newConfig);
    private:
    };
}