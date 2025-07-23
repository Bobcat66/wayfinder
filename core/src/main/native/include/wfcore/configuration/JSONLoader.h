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

    // TODO: refactor this
    class JSONLoader {
    public:
        JSONLoader() = default;
        JSONLoader(
            std::filesystem::path resourceDir,
            std::filesystem::path localDir
        );
        // Loads a JSON object from the resources directory
        WFResult<JSON> loadResourceJSON(const std::string& subdirName, const std::string& filename) const;
        WFResult<JSON> loadLocalJSON(const std::string& subdirName, const std::string& filename) const;
        // This will completely overrwrite the file if it already exists, use with caution
        WFStatusResult storeLocalJSON(const std::string& subdirName, const std::string& filename, const JSON& jobject) const;
        WFResult<std::vector<std::string>> enumerateResourceSubdir(const std::string& subdirName) const;
        WFResult<std::vector<std::string>> enumerateLocalSubdir(const std::string& subdirName) const;
        std::vector<std::string> enumerateResourceSubdirs() const;
        std::vector<std::string> enumerateLocalSubdirs() const;
        bool resourceSubdirExists(const std::string& name) const;
        bool localSubdirExists(const std::string& name) const;
        WFStatusResult assignLocalSubdir(const std::string& subdirName,const std::filesystem::path& subdirRelpath);
        WFStatusResult assignResourceSubdir(const std::string& subdirName,const std::filesystem::path& subdirRelpath);
    private:
        // Path to the directory containing wayfinder resources (models, field configs, etc.)
        std::filesystem::path resourceDir_;
        // Path to the directory containing local configuration (hardware config, vision worker config)
        std::filesystem::path localDir_;

        std::unordered_map<std::string,std::filesystem::path> localSubdirs;
        std::unordered_map<std::string,std::filesystem::path> resourceSubdirs;
    };
}