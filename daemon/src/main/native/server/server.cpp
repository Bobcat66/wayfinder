/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2025 Jesse Kane, Valentina Carcassi
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

#include "server/server.h"
#include "server/server_utils.h"
#include "server/basic_handlers.h"
#include "server/env_handlers.h"
#include "server/dynamic_handlers.h"
#include "wfcore/common/envutils.h"
#include <cctype>
#include <format>
#include <vector>
#include <string>
#include <optional>
#include <cstdlib>
#include "wfcore/common/wfassert.h"
#include "wfcore/common/json_utils.h"
#include "wfd.h"
#include "wfcore/hardware/CameraConfiguration.h"
#include "wfcore/common/jval_compat.h"
#include "jval/CameraConfig.jval.hpp"
#include "jval/VisionWorkerConfig.jval.hpp"
#include <iostream>

namespace impl {
    using namespace wfsrv;

    void configure_env_endpoints(httplib::Server& srv, wf::WFOrchestrator& orch) {
        //api/env/live/team GET OPTIONS
        srv.Get("/api/env/live/team",makeHandler_int_GET(
            [&orch](){return std::format("{}",orch.getSystemConfig().team);}
        ));
        srv.Options("/api/env/live/team",makeHandler_OPTIONS({"GET","OPTIONS"}));

        //api/env/live/devname GET OPTIONS
        srv.Get("/api/env/live/devname",makeHandler_string_GET(
            [&orch](){return orch.getSystemConfig().device_name;}
        ));
        srv.Options("/api/env/live/devname",makeHandler_OPTIONS({"GET","OPTIONS"}));

        //api/env/live/slam GET OPTIONS
        srv.Get("/api/env/live/slam",makeHandler_bool_GET(
            [&orch](){return std::format("{}",orch.getSystemConfig().slam_server);}
        ));
        srv.Options("/api/env/live/slam",makeHandler_OPTIONS({"GET","OPTIONS"}));

        //api/env/local/team GET PUT OPTIONS
        srv.Get("/api/env/local/team",makeHandler_env_stored_GET("WF_TEAM"));
        srv.Put("/api/env/local/team",makeHandler_int_PUT(
            [](const std::string& content) -> std::optional<std::string> {
                auto res = invoke_wfcfg("putenv","WF_TEAM",content);
                if (res == 0) return std::nullopt;
                return std::format("Error while invoking wfcfg subprocess: {}",res);
            }
        ));
        srv.Options("/api/env/local/team",makeHandler_OPTIONS({"GET","PUT","OPTIONS"}));

        //api/env/local/devname GET PUT OPTIONS
        srv.Get("/api/env/local/devname",makeHandler_env_stored_GET("WF_DEVICE_NAME"));
        srv.Put("/api/env/local/devname",makeHandler_string_PUT(
            [](const std::string& content) -> std::optional<std::string> {
                auto res = invoke_wfcfg("putenv","WF_DEVICE_NAME",content);
                if (res == 0) return std::nullopt;
                return std::format("Error while invoking wfcfg subprocess: {}",res);
            }
        ));
        srv.Options("/api/env/local/devname",makeHandler_OPTIONS({"GET","PUT","OPTIONS"}));

        //api/env/local/slam GET PUT OPTIONS
        srv.Get("/api/env/local/slam",makeHandler_env_stored_GET("WF_SLAM_SERVER"));
        srv.Put("/api/env/local/slam",makeHandler_bool_PUT(
            [](const std::string& content) -> std::optional<std::string> {
                auto res = invoke_wfcfg("putenv","WF_SLAM_SERVER",content);
                if (res == 0) return std::nullopt;
                return std::format("Error while invoking wfcfg subprocess: {}",res);
            }
        ));
        srv.Options("/api/env/local/slam",makeHandler_OPTIONS({"GET","PUT","OPTIONS"}));
    }

    void configure_network_endpoints(httplib::Server& srv, wf::WFOrchestrator& orch) {

    }

    void configure_local_endpoints(httplib::Server& srv, wf::WFOrchestrator& orch) {

        //api/local/hardware GET OPTIONS
        srv.Get("/api/local/hardware",makeHandler_enum_local_subdir("hardware",orch));
        srv.Options("/api/local/hardware",makeHandler_OPTIONS({"OPTIONS","GET"}));
        
        //api/local/hardware/*.json GET PUT PATCH DELETE OPTIONS
        srv.Get("/api/local/hardware/([^/]+.json)",makeHandler_local_file_GET(
            "hardware",
            [](const httplib::Request& req){ return req.matches[1].str(); },
            orch
        ));
        srv.Put("/api/local/hardware/([^/]+.json)",makeHandler_local_file_PUT(
            "hardware",
            [](const httplib::Request& req){ return req.matches[1].str(); },
            orch,
            jval::asLambda(jval::get_CameraConfig_validator())
        ));
        srv.Patch("/api/local/hardware/([^/]+.json)",makeHandler_local_file_PATCH(
            "hardware",
            [](const httplib::Request& req){ return req.matches[1].str(); },
            orch,
            jval::asLambda(jval::get_CameraConfig_validator())
        ));
        srv.Delete("/api/local/hardware/([^/]+.json)",makeHandler_local_file_DELETE(
            "hardware",
            [](const httplib::Request& req){ return req.matches[1].str(); },
            orch
        ));
        srv.Options("/api/local/hardware/([^/]+.json)",makeHandler_OPTIONS({"OPTIONS","GET","PUT","PATCH","DELETE"}));

        //api/local/pipelines GET OPTIONS
        
        srv.Get("/api/local/pipelines",makeHandler_enum_local_subdir("pipelines",orch));
        srv.Options("/api/local/pipelines",makeHandler_OPTIONS({"OPTIONS","GET"}));
        
        //api/local/hardware/*.json GET PUT PATCH OPTIONS DELETE
        srv.Get("/api/local/pipelines/([^/]+.json)",makeHandler_local_file_GET(
            "pipelines",
            [](const httplib::Request& req){ return req.matches[1].str(); },
            orch
        ));
        srv.Put("/api/local/pipelines/([^/]+.json)",makeHandler_local_file_PUT(
            "pipelines",
            [](const httplib::Request& req){ return req.matches[1].str(); },
            orch,
            jval::asLambda(jval::get_VisionWorkerConfig_validator())
        ));
        srv.Patch("/api/local/pipelines/([^/]+.json)",makeHandler_local_file_PATCH(
            "hardware",
            [](const httplib::Request& req){ return req.matches[1].str(); },
            orch,
            jval::asLambda(jval::get_VisionWorkerConfig_validator())
        ));
        srv.Delete("/api/local/pipelines/([^/]+.json)",makeHandler_local_file_DELETE(
            "pipelines",
            [](const httplib::Request& req){ return req.matches[1].str(); },
            orch
        ));
        srv.Options("/api/local/pipelines/([^/]+.json)",makeHandler_OPTIONS({"OPTIONS","GET","PUT","PATCH","DELETE"}));
    }

    void configure_resource_endpoints(httplib::Server& srv, wf::WFOrchestrator& orch) {

        //api/resources/fields GET OPTIONS
        srv.Get("/api/resources/fields",makeHandler_enum_resource_subdir("fields",orch));
        srv.Options("/api/resources/fields",makeHandler_OPTIONS({"OPTIONS","GET"}));

        //api/resources/fields/*.json GET OPTIONS
        srv.Get("/api/resources/fields/([^/]+.json)",makeHandler_resource_file_GET(
            "resources",
            [](const httplib::Request& req){ return req.matches[1].str(); },
            orch
        ));
        srv.Options("/api/resources/fields/([^/]+.json)",makeHandler_OPTIONS({"OPTIONS","GET"}));

        //api/resources/models GET OPTIONS
        srv.Get("/api/resources/models",makeHandler_enum_resource_subdir("models",orch));
        srv.Options("/api/resources/models",makeHandler_OPTIONS({"OPTIONS","GET"}));
    }
}

namespace wfsrv {
    HTTPServer::HTTPServer(wf::WFOrchestrator& orch)
    : orch_(orch){
        srv_.new_task_queue = [] {
            // 4 worker threads
            return new httplib::ThreadPool(4);
        };
        impl::configure_env_endpoints(srv_,orch_);
        impl::configure_local_endpoints(srv_,orch_);
        impl::configure_resource_endpoints(srv_,orch_);
        
    }

    void HTTPServer::start() {
        if (listenerThread.joinable())
            return; // already running

        listenerThread = std::jthread([&](std::stop_token st){
            std::stop_callback cb(st, [&](){
                srv_.stop();
            });

            auto apiPort = wf::env::getInt("WF_API_PORT",true);
            if (!apiPort) return;

            // TODO: Handle .listen() failures?
            srv_.listen("0.0.0.0",apiPort.value());
        });
    }

    void HTTPServer::stop() {
        if (listenerThread.joinable())
            listenerThread.request_stop();
    }
}