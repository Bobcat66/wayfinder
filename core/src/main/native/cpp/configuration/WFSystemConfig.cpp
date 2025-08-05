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

#include "wfcore/configuration/WFSystemConfig.h"
#include "wfcore/common/envutils.h"
#include <functional>

namespace impl {
    using namespace wf;

    template <typename T>
    WFResult<T> getEnv(std::string key,std::function<std::optional<T>(const char*)> getter) {
        std::optional<T> opt = getter(key.c_str());
        return opt 
            ? WFResult<T>::success(std::move(opt.value()))
            : WFResult<T>::failure(
                static_cast<WFStatus>(env::getError()),
                "Failed to retrieve {}", key
            );
    }
}

namespace wf {
    namespace fs = std::filesystem;
    WFResult<WFSystemConfig> WFSystemConfig::getFromEnv() {

        auto devname_res = impl::getEnv<std::string>(
            "WF_DEVICE_NAME",[](const char* key){return env::getVar(key,false);}
        );
        if (!devname_res) 
            return WFResult<WFSystemConfig>::propagateFail(devname_res);

        auto team_res = impl::getEnv<int>(
            "WF_TEAM",[](const char* key){return env::getInt(key,false);}
        );
        if (!team_res) 
            return WFResult<WFSystemConfig>::propagateFail(team_res);
        
        auto slam_server_res = impl::getEnv<bool>(
            "WF_SLAM_SERVER",[](const char* key){return env::getBool(key,false);}
        );
        if (!slam_server_res) 
            return WFResult<WFSystemConfig>::propagateFail(slam_server_res);

        auto nt_server_res = impl::getEnv<bool>(
            "WF_NT_SERVER",[](const char* key){return env::getBool(key,false);}
        );
        if (!nt_server_res) 
            return WFResult<WFSystemConfig>::propagateFail(nt_server_res);

        auto local_path_res = impl::getEnv<std::string>(
            "WF_LOCAL",[](const char* key){return env::getVar(key,false);}
        );
        if (!local_path_res) 
            return WFResult<WFSystemConfig>::propagateFail(local_path_res);

        auto resource_path_res = impl::getEnv<std::string>(
            "WF_DEVICE_NAME",[](const char* key){return env::getVar(key,false);}
        );
        if (!resource_path_res) 
            return WFResult<WFSystemConfig>::propagateFail(resource_path_res);

        auto models_rsubdir_res = impl::getEnv<std::string>(
            "WF_MODELS_RSUBDIR",[](const char* key){return env::getVar(key,false);}
        );
        if (!models_rsubdir_res) 
            return WFResult<WFSystemConfig>::propagateFail(models_rsubdir_res);

        auto fields_rsubdir_res = impl::getEnv<std::string>(
            "WF_FIELDS_RSUBDIR",[](const char* key){return env::getVar(key,false);}
        );
        if (!fields_rsubdir_res) 
            return WFResult<WFSystemConfig>::propagateFail(fields_rsubdir_res);

        auto hardware_lsubdir_res = impl::getEnv<std::string>(
            "WF_HARDWARE_LSUBDIR",[](const char* key){return env::getVar(key,false);}
        );
        if (!hardware_lsubdir_res) 
            return WFResult<WFSystemConfig>::propagateFail(hardware_lsubdir_res);
        
        
        auto pipeline_lsubdir_res = impl::getEnv<std::string>(
            "WF_PIPELINE_LSUBDIR",[](const char* key){return env::getVar(key,false);}
        );
        if (!pipeline_lsubdir_res) 
            return WFResult<WFSystemConfig>::propagateFail(pipeline_lsubdir_res);
        
        return WFResult<WFSystemConfig>::success(
            std::in_place,
            std::move(devname_res.value()),
            team_res.value(),
            slam_server_res.value(),
            nt_server_res.value(),
            PathConfig{
                fs::path(std::move(local_path_res.value())),
                fs::path(std::move(resource_path_res.value())),
                fs::path(std::move(models_rsubdir_res.value())),
                fs::path(std::move(fields_rsubdir_res.value())),
                fs::path(std::move(pipeline_lsubdir_res.value())),
                fs::path(std::move(hardware_lsubdir_res.value()))
            }
        );
    }
}