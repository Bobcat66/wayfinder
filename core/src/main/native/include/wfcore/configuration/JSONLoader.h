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
#include <unordered_map>
#include <optional>
#include <vector>
#include "wfcore/common/json_utils.h"

namespace wf {

    enum class JSONLoaderStatus {
        Ok,
        FileNotOpened,
        SchemaViolation,
        JSONParseError
    };

    // TODO: refactor this
    class JSONLoader : public LoggedStatusfulObject<JSONLoaderStatus,JSONLoaderStatus::Ok> {
    public:
        JSONLoader(std::filesystem::path rootPath);
        // Loads a JSON object from the resources directory
        std::optional<JSONObject> loadJSONObject(const std::string& subdirName, const std::string& filename) const;
        bool storeJSONObject(const std::string& subdirName, const std::string& filename, const JSONObject& jobject) const;
        std::vector<std::string> enumerateFiles(const std::string& subdirName) const;
        bool addSubdirectory(const std::string& subdirName, std::filesystem::path relpath);
    private:
        // Path to the directory containing the local/ and resource/ directories.
        // Usually this will be wayfinder's root directory
        const std::filesystem::path rootPath_;
        
        std::unordered_map<std::string,std::filesystem::path> subdirectories;
    };
}