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

#include "wfcore/configuration/ConfigLoader.h"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace wf {

    using enum ConfigLoaderStatus;

    ConfigLoader::ConfigLoader(fs::path localDirPath, fs::path resourceDirPath) 
    : LoggedStatusfulObject<ConfigLoaderStatus,ConfigLoaderStatus::Ok>("ConfigLoader",LogGroup::System)
    , localDirPath_(localDirPath)
    , resourceDirPath_(resourceDirPath) {}

    std::optional<CameraConfiguration> ConfigLoader::loadCameraConfig(const std::string& filename) const noexcept {
        std::ifstream configFile(localDirPath_ / "cameras" / filename);
        if (!configFile) {
            this->reportError(
                FileNotOpened,
                "Failed to open file {}/cameras/{}",
                localDirPath_.string(),
                filename
            );
            return std::nullopt;
        }
        return std::nullopt;
    }

    std::optional<VisionWorkerConfig> ConfigLoader::loadWorkerConfig(const std::string& filename) const noexcept {
        return std::nullopt; // Placeholder so the damn thing builds
    }

    bool ConfigLoader::storeVisionWorkerConfig(const std::string& filename,const VisionWorkerConfig& config) const noexcept {
        return true; // Placeholder
    }

    bool ConfigLoader::storeCameraConfig(const std::string& filename, const CameraConfiguration& config) const noexcept {
        return true; // Placeholder
    }

    std::vector<std::string> ConfigLoader::searchForCameraConfigFiles() const noexcept {
        return {}; // Placeholder
    }

    std::vector<std::string> ConfigLoader::searchForVisionWorkerConfigFiles() const noexcept {
        return {}; // Placeholder
    }

    std::optional<ApriltagField> ConfigLoader::loadField(const std::string& filename) const noexcept {
        return std::nullopt; // Placeholder
    }
}