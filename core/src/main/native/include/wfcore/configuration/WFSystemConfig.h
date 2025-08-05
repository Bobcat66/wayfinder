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

#include <string>
#include <filesystem>
#include "wfcore/common/status.h"

namespace wf {

    struct PathConfig {
        std::filesystem::path local_path;
        std::filesystem::path resource_path;
        std::filesystem::path models_rsubdir;
        std::filesystem::path fields_rsubdir;
        std::filesystem::path pipeline_lsubdir;
        std::filesystem::path hardware_lsubdir;
    };

    struct WFSystemConfig {
        std::string device_name;
        int team;
        bool slam_server;
        bool nt_server;
        PathConfig paths;

        static WFResult<WFSystemConfig> getFromEnv();
    };

}