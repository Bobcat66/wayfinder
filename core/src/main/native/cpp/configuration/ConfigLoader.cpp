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
#include <unordered_map>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace impl {
    using namespace wf;
    static const std::unordered_map<CameraBackend,std::string> backendMap = {
        {CameraBackend::CSCORE,"CSCORE"},
        {CameraBackend::REALSENSE,"REALSENSE"},
        {CameraBackend::GSTREAMER,"GSTREAMER"},
        {CameraBackend::LIBCAMERA,"LIBCAMERA"}
    };

    template <typename KeyType, typename ValType>
    std::optional<ValType> searchMapByKey(const std::unordered_map<KeyType,ValType>& map,const KeyType& key) {
        auto it = map.find(key);
        if (it == map.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    template <typename KeyType, typename ValType>
    std::optional<KeyType> searchMapByValue(const std::unordered_map<KeyType,ValType>& map,const ValType& val) {
        for (const auto& [key,value] : map) {
            if (value == val) return key;
        }
        return std::nullopt;
    }
}
namespace wf {

    using enum ConfigLoaderStatus;

    ConfigLoader::ConfigLoader(fs::path localDirPath, fs::path resourceDirPath) 
    : LoggedStatusfulObject<ConfigLoaderStatus,ConfigLoaderStatus::Ok>("ConfigLoader",LogGroup::System)
    , localDirPath_(localDirPath)
    , resourceDirPath_(resourceDirPath) {}

    std::optional<CameraConfiguration> ConfigLoader::loadCameraConfig(const std::string& filename) const noexcept {

        this->logger()->info(
            "Loading camera configuration from {}/cameras/{}",
            localDirPath_.string(),
            filename
        );
        // Load file
        std::ifstream configFile(localDirPath_ / "cameras" / filename);
        if (!configFile) {
            this->reportError(
                FileNotOpened,
                "Failed to open file {}",
                filename
            );
            return std::nullopt;
        }

        // Parse JSON
        json jsonData;
        try {
            configFile >> jsonData;
        } catch (const json::parse_error& e) {
            this->reportError(
                JSONParseError,
                "Error while parsing {}: {}",
                filename,
                e.what()
            );
            return std::nullopt;
        }

        // Retrieve devpath
        if (!jsonData.contains("devpath")) {
            this->reportError(
                SchemaViolation,
                "{} does not contain required field 'devpath'",
                filename
            );
            return std::nullopt;
        }
        if (!jsonData["devpath"].is_string()) {
            this->reportError(
                SchemaViolation,
                "{} contains field 'devpath', but it is not a string",
                filename
            );
            return std::nullopt;
        }
        auto devpath = jsonData["devpath"].get<std::string>();

        // Retrieve backend
        if (!jsonData.contains("backend"))

        
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