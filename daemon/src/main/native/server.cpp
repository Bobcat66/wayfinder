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

#include "server.h"
#include "wfcore/common/envutils.h"
#include <cctype>
#include <format>
#include <vector>
#include <string>
#include <filesystem>
#include <optional>
#include <sstream>
#include <cstdlib>
#include <sys/wait.h>
#include "wfcore/common/wfassert.h"
#include <type_traits>
#include "wfcore/common/json_utils.h"
#include <iomanip>
#include "wfd.h"
#include <functional>
#include "wfcore/hardware/CameraConfiguration.h"
#include "wfcore/common/jval_compat.h"
#include "jval/CameraConfig.jval.hpp"
#include "jval/VisionWorkerConfig.jval.hpp"
#include "jval/JSONPatchRFC6902.jval.hpp"
#include <optional>

namespace impl {
    namespace fs = std::filesystem;
    // Returns if a string is alphanumeric
    bool isAlnumStr(const std::string& msg) {
        for (const char c : msg) {
            if (!std::isalnum(static_cast<unsigned char>(c))) {
                return false;
            }
        }
        return true;
    }
    bool isDigitStr(const std::string& msg) {
        for (const char c : msg) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                return false;
            }
        }
        return true;
    }
    bool isBoolStr(const std::string& msg) {
        return ((msg == "true") || (msg == "false"));
    }

    std::optional<std::string> getScriptsPath() {
        // for lazy one-time evaluation
        static bool loaded = false;
        static bool badpath = false;
        static std::string spath;
        if (badpath) return std::nullopt;

        if (!loaded) {
            // lazily fetch scripts path from environment
            auto spathstropt = wf::env::getVar("WF_PATH");
            if (!spathstropt) {
                badpath = true;
                return std::nullopt;
            }
            if (!fs::exists(fs::path(spathstropt.value()))) {
                badpath = true;
                return std::nullopt;
            }
            spath = spathstropt.value();
            loaded = true;
            return spath;
        }

        return spath;
    }

    std::string join(const std::vector<std::string>& vec, const std::string& sep) {
        std::ostringstream oss;
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i != 0) oss << sep;
            oss << vec[i];
        }
        return oss.str();
    }

    template <typename... Args>
    int invoke_wfcfg(Args&&... args) {
        auto scriptPathOpt = getScriptsPath();
        if (!scriptPathOpt) return -1;

        std::ostringstream cmd_oss;
        cmd_oss << scriptPathOpt.value() << "/wfcfg.py";
        ((cmd_oss << " " << std::quoted(args)), ...);

        int rc = std::system(cmd_oss.str().c_str());
        
        if (rc == -1) return -1;          // system() failed
        if (WIFEXITED(rc)) return WEXITSTATUS(rc);
        return -1;                         // child did not exit normally
    }

    constexpr std::string_view getStatusMsg(int status) {
        switch (status) {
            // 1xx Informational
            case 100: return "Continue";
            case 101: return "SwitchingProtocols";
            case 102: return "Processing";
            case 103: return "EarlyHints";

            // 2xx Success
            case 200: return "OK";
            case 201: return "Created";
            case 202: return "Accepted";
            case 203: return "NonAuthoritativeInformation";
            case 204: return "NoContent";
            case 205: return "ResetContent";
            case 206: return "PartialContent";
            case 207: return "MultiStatus";
            case 208: return "AlreadyReported";
            case 226: return "IMUsed";

            // 3xx Redirection
            case 300: return "MultipleChoices";
            case 301: return "MovedPermanently";
            case 302: return "Found";
            case 303: return "SeeOther";
            case 304: return "NotModified";
            case 307: return "TemporaryRedirect";
            case 308: return "PermanentRedirect";

            // 4xx Client Error
            case 400: return "BadRequest";
            case 401: return "Unauthorized";
            case 402: return "PaymentRequired";
            case 403: return "Forbidden";
            case 404: return "NotFound";
            case 405: return "MethodNotAllowed";
            case 406: return "NotAcceptable";
            case 407: return "ProxyAuthenticationRequired";
            case 408: return "RequestTimeout";
            case 409: return "Conflict";
            case 410: return "Gone";
            case 411: return "LengthRequired";
            case 412: return "PreconditionFailed";
            case 413: return "PayloadTooLarge";
            case 414: return "URITooLong";
            case 415: return "UnsupportedMediaType";
            case 416: return "RangeNotSatisfiable";
            case 417: return "ExpectationFailed";
            case 418: return "ImATeapot";
            case 421: return "MisdirectedRequest";
            case 422: return "UnprocessableEntity";
            case 423: return "Locked";
            case 424: return "FailedDependency";
            case 425: return "TooEarly";
            case 426: return "UpgradeRequired";
            case 428: return "PreconditionRequired";
            case 429: return "TooManyRequests";
            case 431: return "RequestHeaderFieldsTooLarge";
            case 451: return "UnavailableForLegalReasons";

            // 5xx Server Error
            case 500: return "InternalServerError";
            case 501: return "NotImplemented";
            case 502: return "BadGateway";
            case 503: return "ServiceUnavailable";
            case 504: return "GatewayTimeout";
            case 505: return "HTTPVersionNotSupported";
            case 506: return "VariantAlsoNegotiates";
            case 507: return "InsufficientStorage";
            case 508: return "LoopDetected";
            case 510: return "NotExtended";
            case 511: return "NetworkAuthenticationRequired";

            default: 
                if (std::is_constant_evaluated()){
                    throw "Unexpected HTTP status code!";
                } else {
                    return "UnknownStatusCode";
                }
        }
    }

    // For when status is known at compile time
    template <int Status, typename... Args>
    std::string getErrorResponse(std::string_view fmt,Args&&... args) {
        constexpr auto statusMsg = getStatusMsg(Status);
        return std::format(
            "{{\"status\":{},\"error\":\"{}\",\"message\":\"{}\"}}",
            Status,
            statusMsg,
            std::vformat(fmt,std::make_format_args(args...))
        );
    }

    // For when status is known at runtime
    template <typename... Args>
    std::string getErrorResponse(int status, std::string_view fmt,Args&&... args) {
        return std::format(
            "{{\"status\":{},\"error\":\"{}\",\"message\":\"{}\"}}",
            status,
            getStatusMsg(status),
            std::vformat(fmt,std::make_format_args(args...))
        );
    }

    void setContent(httplib::Response& res, const std::string& content) {
        res.set_content(
            content,
            "application/json"
        );
    }

    // If the return string is present, the operation failed and the server will return error 500
    using genericPutConsumer = std::function<std::optional<std::string>(const std::string&)>;

    // ConsumerCallable should be a callable which takes a const std::string& and returns an std::optional<std::string>>
    // If the return string is present, the operation failed and the server will return error 500
    // ValidatorCallable should be a callable which takes a const std::string& and returns a bool
    template <typename ConsumerCallable, typename ValidatorCallable>
    auto makeHandler_generic_PUT(
        ConsumerCallable consumer,
        ValidatorCallable validator,
        std::string errmsg422 = ""
    ){
        return [consumer,validator,errmsg422](const httplib::Request& req, httplib::Response& res){
            if (!validator(req.body)) {
                res.status = 422;
                setContent(res, getErrorResponse<422>(errmsg422));
                return;
            }
            std::optional<std::string> cerrmsg = consumer(req.body);
            if (cerrmsg) {
                res.status = 500;
                setContent(res, getErrorResponse<500>(cerrmsg.value()));
            } else {
                res.status = 204;
            }
        };
    }

    // GetterCallable should be a callable taking void args and returning a string-like object
    template <typename GetterCallable>
    auto makeHandler_generic_GET(GetterCallable resourceGetter) {
        return [resourceGetter](const httplib::Request& req, httplib::Response& res){
            res.status = 200;
            setContent(res, resourceGetter());
        };
    }

    auto makeHandler_OPTIONS(std::vector<std::string> supportedMethods) {
        return [supportedMethods](const httplib::Request& req, httplib::Response& res){
            res.set_header("X-Clacks-Overhead","GNU Terry Pratchett");
            res.set_header("Allow",join(supportedMethods,", "));
            res.status = 204;
        };
    }

    template <wf::WFResult<wf::JSON> (wf::ResourceManager::*JSONLoader)(const std::string&,const std::string&) const, typename FilenameGetter>
    auto makeHandler_json_GET(std::string subdir, FilenameGetter filenameGetter, wf::WFOrchestrator& orch) {
        return [&orch,subdir,filenameGetter](const httplib::Request& req, httplib::Response& res){
            std::string filename;
            try {
                filename = filenameGetter(req);
            } catch (...) {
                res.status = 500;
                setContent(res, getErrorResponse<500>("An unknown exception occurred while parsing file name"));
                return;
            }
            auto resource_res = (orch.getResourceManager().*JSONLoader)(subdir,filename);
            if (!resource_res) {
                switch (resource_res.status()) {
                    case wf::WFStatus::FILE_NOT_FOUND:
                    case wf::WFStatus::CONFIG_SUBDIR_NOT_FOUND:
                        res.status = 404;
                        setContent(res, getErrorResponse<404>(resource_res.what()));
                        return;
                    default:
                        res.status = 500;
                        setContent(res, getErrorResponse<500>(resource_res.what()));
                        return;
                }
            }
            auto content = resource_res.value().dump();
            res.status = 200;
            setContent(res, content);
            return;
        };
    }

    // FilenameGetter should take a const http Request and return a string
    template <typename FilenameGetter>
    auto makeHandler_local_json_GET(std::string subdir, FilenameGetter filenameGetter, wf::WFOrchestrator& orch) {
        return makeHandler_json_GET<&wf::ResourceManager::loadLocalJSON>(subdir,filenameGetter,orch);
    }

    // FilenameGetter should take a const http Request and return a string
    template <typename FilenameGetter>
    auto makeHandler_resource_json_GET(std::string subdir, FilenameGetter filenameGetter, wf::WFOrchestrator& orch) {
        return makeHandler_json_GET<&wf::ResourceManager::loadResourceJSON>(subdir,filenameGetter,orch);
    }

    // FilenameGetter should take a const http Request and return a string
    // JSONValidatorCallable should be a callable which takes a const JSON& object and returns a JVResult or WFResult (JVResult is preferred)
    template <typename JSONValidatorCallable, typename FilenameGetter>
    auto makeHandler_local_json_PUT(std::string subdir, FilenameGetter filenameGetter, wf::WFOrchestrator& orch, JSONValidatorCallable validator) {
        return [&orch,subdir,filenameGetter,validator](const httplib::Request& req, httplib::Response& res){
            std::string filename;
            try {
                filename = filenameGetter(req);
            } catch (...) {
                res.status = 500;
                setContent(res, getErrorResponse<500>("An unknown exception occurred while parsing file name"));
                return;
            }
            auto resolve_res = orch.getResourceManager().resolveLocalFile(subdir,filename);
            std::function<void(void)> successHandler; // Handler for successful cases
            switch (resolve_res.status()) {
                // Determines successful status code
                case wf::WFStatus::OK:
                    if (!resolve_res) {
                        // sanity check. This path is only triggered when WFStatus's contract is violated and should never happen
                        res.status = 500;
                        setContent(res, getErrorResponse<500>("Attempting to resolve file caused an invariant violation (an invalid WFResult reported a nominal status)"));
                        return;
                    }
                    successHandler = [&res](){ res.status = 204; };
                    break;
                case wf::WFStatus::FILE_NOT_FOUND: 
                    successHandler = [&res,&filename,&subdir](){
                        res.status = 201;
                        res.set_header("Location", std::format("/api/local/{}/{}",subdir,filename));
                    };
                    break;
                case wf::WFStatus::CONFIG_SUBDIR_NOT_FOUND:
                    res.status = 404;
                    setContent(res, getErrorResponse<404>(resolve_res.what()));
                    return;
                default:
                    res.status = 500;
                    setContent(res, getErrorResponse<500>(resolve_res.what()));
                    return;
            }
            // Implementation
            wf::JSON req_jobject;
            try {
                req_jobject = wf::JSON::parse(req.body);
            } catch (const wf::JSON::parse_error& e) {
                res.status = 400;
                setContent(res, getErrorResponse<400>("Request did not contain valid JSON"));
                return;
            }
            auto valid_res = validator(req_jobject);
            if (!valid_res) {
                res.status = 422;
                setContent(res, getErrorResponse<422>(valid_res.what()));
                return;
            }
            auto store_res = orch.getResourceManager().storeLocalJSON(subdir,filename,req_jobject);
            if (!store_res) {
                res.status = 500;
                setContent(res, getErrorResponse<500>(store_res.what()));
                return;
            }

            successHandler();
            return;
        };
    }
    
    template <typename JSONValidatorCallable, typename FilenameGetter>
    auto makeHandler_local_json_PATCH(std::string subdir, FilenameGetter filenameGetter, wf::WFOrchestrator& orch, JSONValidatorCallable validator) {
        return [&orch,subdir,filenameGetter,validator](const httplib::Request& req, httplib::Response& res) {
            std::string filename;
            try {
                filename = filenameGetter(req);
            } catch (...) {
                res.status = 500;
                setContent(res, getErrorResponse<500>("An unknown exception occurred while parsing file name"));
                return;
            }
            auto json_res = orch.getResourceManager().loadLocalJSON(subdir, filename);
            if (!json_res) {
                switch (json_res.status()) {
                    case wf::WFStatus::CONFIG_SUBDIR_NOT_FOUND:
                    case wf::WFStatus::FILE_NOT_FOUND:
                        res.status = 404;
                        setContent(res, getErrorResponse<404>(json_res.what()));
                        return;
                    default:
                        res.status = 500;
                        setContent(res, getErrorResponse<500>(json_res.what()));
                        return;
                }
            }
            auto jobject = std::move(json_res.value());
            wf::JSON patch;
            try {
                patch = wf::JSON::parse(req.body);
            } catch (const wf::JSON::parse_error& e) {
                res.status = 400;
                setContent(res, getErrorResponse<400>("Request did not contain valid JSON"));
                return;
            }
            auto patch_valid_res = (*jval::get_JSONPatchRFC6902_validator())(patch);
            if (!patch_valid_res) {
                res.status = 422;
                setContent(res, getErrorResponse<422>(patch_valid_res.what()));
                return;
            }

            try {
                jobject.patch(patch);
            } catch (const wf::JSON::exception& e) {
                res.status = 422;
                setContent(res, getErrorResponse<422>(e.what()));
                return;
            };

            auto jobject_valid_res = validator(jobject);
            if (!jobject_valid_res) {
                res.status = 422;
                setContent(res, getErrorResponse<422>(
                    "Patched json violated schema: {}",
                    jobject_valid_res.what()
                ));
                return;
            }

            auto store_res = orch.getResourceManager().storeLocalJSON(subdir,filename,jobject);
            if (!store_res) {
                res.status = 500;
                setContent(res, getErrorResponse<500>(store_res.what()));
                return;
            }

            res.status = 204;
            return;
        };
    }

    // Overload for when we don't need any validation for a local JSON putter
    template <typename FilenameGetter>
    auto makeHandler_local_json_PUT(std::string subdir, FilenameGetter filenameGetter, wf::WFOrchestrator& orch) {
        return makeHandler_local_json_PUT(subdir,filenameGetter,orch,jval::asLambda(jval::getNullValidator()));
    }

    template <typename FilenameGetter>
    auto makeHandler_local_json_DELETE(std::string subdir, FilenameGetter filenameGetter, wf::WFOrchestrator& orch) {
        return [&orch,subdir,filenameGetter](const httplib::Request& req, httplib::Response& res){
            std::string filename;
            try {
                filename = filenameGetter(req);
            } catch (...) {
                res.status = 500;
                setContent(res, getErrorResponse<500>("An unknown exception occurred while parsing file name"));
                return;
            }
            auto deleteRes = orch.getResourceManager().deleteLocalJSON(subdir,filename);
            switch (deleteRes.status()) {
                case wf::WFStatus::OK:
                    if (!deleteRes) {
                        // sanity check. This path is only triggered when WFStatus's contract is violated and should never happen
                        res.status = 500;
                        setContent(res, getErrorResponse<500>("Attempting to delete file caused an invariant violation (an invalid WFResult reported a nominal status)"));
                        return;
                    }
                    res.status = 204;
                    return;
                case wf::WFStatus::CONFIG_SUBDIR_NOT_FOUND:
                case wf::WFStatus::FILE_NOT_FOUND:
                    res.status = 404;
                    setContent(res, getErrorResponse<404>(deleteRes.what()));
                    return;
                default:
                    res.status = 500;
                    setContent(res, getErrorResponse<500>(deleteRes.what()));
                    return;
            }
        };
    }

    // Both possible enumerators are const methods of resourceManager
    template <wf::WFResult<std::vector<std::string>>(wf::ResourceManager::*Enumerator)(const std::string&) const>
    auto makeHandler_enum_subdir(std::string subdir, wf::WFOrchestrator& orch) {
        return [&orch,subdir](const httplib::Request& req, httplib::Response& res){
            auto files_res = (orch.getResourceManager().*Enumerator)(subdir);
            if (!files_res) {
                // we use a switch and not an if just in case we want to add more cases in the future
                switch (files_res.status()) {
                    case wf::WFStatus::CONFIG_SUBDIR_NOT_FOUND:
                        res.status = 404;
                        setContent(res, getErrorResponse<404>(files_res.what()));
                        return;
                    default:
                        res.status = 500;
                        setContent(res, getErrorResponse<500>(files_res.what()));
                        return;
                }
            }
            try {
                wf::JSON files_jobject = files_res.value();
                res.status = 200;
                setContent(res, files_jobject.dump());
                return;
            } catch (const wf::JSON::exception& e) {
                res.status = 500;
                setContent(res, getErrorResponse<500>(e.what()));
                return;
            }
        };
    }

    auto makeHandler_enum_local_subdir(std::string subdir, wf::WFOrchestrator& orch) {
        return makeHandler_enum_subdir<&wf::ResourceManager::enumerateLocalSubdir>(subdir,orch);
    }

    auto makeHandler_enum_resource_subdir(std::string subdir, wf::WFOrchestrator& orch) {
        return makeHandler_enum_subdir<&wf::ResourceManager::enumerateResourceSubdir>(subdir,orch);
    }

    void configure_env_endpoints(httplib::Server& srv, wf::WFOrchestrator& orch) {
        //api/env/team GET PUT OPTIONS
        srv.Get("/api/env/team",makeHandler_generic_GET(
            [&orch](){return std::format("{}",orch.getSystemConfig().team);}
        ));
        srv.Put("/api/env/team",makeHandler_generic_PUT(
            [](const std::string& content) -> std::optional<std::string> {
                auto res = invoke_wfcfg("putenv","WF_TEAM",content);
                if (res == 0) return std::nullopt;
                return std::format("Error while invoking wfcfg subprocess: {}",res);
            },
            isDigitStr,
            "Content is not a numeric string"
        ));
        srv.Options("/api/env/team",makeHandler_OPTIONS({"GET","PUT","OPTIONS"}));

        //api/env/team GET PUT OPTIONS
        srv.Get("/api/env/devname",makeHandler_generic_GET(
            [&orch](){return orch.getSystemConfig().device_name;}
        ));
        srv.Put("/api/env/devname",makeHandler_generic_PUT(
            [](const std::string& content) -> std::optional<std::string> {
                auto res = invoke_wfcfg("putenv","WF_DEVICE_NAME",content);
                if (res == 0) return std::nullopt;
                return std::format("Error while invoking wfcfg subprocess: {}",res);
            },
            isAlnumStr,
            "Content is not an alphanumeric string"
        ));
        srv.Options("/api/env/slam",makeHandler_OPTIONS({"GET","PUT","OPTIONS"}));

        //api/env/slam GET PUT OPTIONS
        srv.Get("/api/env/slam",makeHandler_generic_GET(
            [&orch](){return std::format("{}",orch.getSystemConfig().slam_server);}
        ));
        srv.Put("/api/env/slam",makeHandler_generic_PUT(
            [](const std::string& content) -> std::optional<std::string> {
                auto res = invoke_wfcfg("putenv","WF_SLAM_SERVER",content);
                if (res == 0) return std::nullopt;
                return std::format("Error while invoking wfcfg subprocess: {}",res);
            },
            isBoolStr,
            "Content is not a boolean string"
        ));
        srv.Options("/api/env/slam",makeHandler_OPTIONS({"GET","PUT","OPTIONS"}));
    }

    void configure_local_endpoints(httplib::Server& srv, wf::WFOrchestrator& orch) {

        //api/local/hardware GET OPTIONS
        srv.Get("/api/local/hardware",makeHandler_enum_local_subdir("hardware",orch));
        srv.Options("/api/local/hardware",makeHandler_OPTIONS({"OPTIONS","GET"}));
        
        //api/local/hardware/*.json GET PUT PATCH DELETE OPTIONS
        srv.Get("/api/local/hardware/([^/]+.json)",makeHandler_local_json_GET(
            "hardware",
            [](const httplib::Request& req){ return req.matches[1].str(); },
            orch
        ));
        srv.Put("/api/local/hardware/([^/]+.json)",makeHandler_local_json_PUT(
            "hardware",
            [](const httplib::Request& req){ return req.matches[1].str(); },
            orch,
            jval::asLambda(jval::get_CameraConfig_validator())
        ));
        srv.Patch("/api/local/hardware/([^/]+.json)",makeHandler_local_json_PATCH(
            "hardware",
            [](const httplib::Request& req){ return req.matches[1].str(); },
            orch,
            jval::asLambda(jval::get_CameraConfig_validator())
        ));
        srv.Delete("/api/local/hardware/([^/]+.json)",makeHandler_local_json_DELETE(
            "hardware",
            [](const httplib::Request& req){ return req.matches[1].str(); },
            orch
        ));
        srv.Options("/api/local/hardware/([^/]+.json)",makeHandler_OPTIONS({"OPTIONS","GET","PUT","PATCH","DELETE"}));

        //api/local/pipelines GET OPTIONS
        
        srv.Get("/api/local/pipelines",makeHandler_enum_local_subdir("pipelines",orch));
        srv.Options("/api/local/pipelines",makeHandler_OPTIONS({"OPTIONS","GET"}));
        
        //api/local/hardware/*.json GET PUT PATCH OPTIONS DELETE
        srv.Get("/api/local/pipelines/([^/]+.json)",makeHandler_local_json_GET(
            "pipelines",
            [](const httplib::Request& req){ return req.matches[1].str(); },
            orch
        ));
        srv.Put("/api/local/pipelines/([^/]+.json)",makeHandler_local_json_PUT(
            "pipelines",
            [](const httplib::Request& req){ return req.matches[1].str(); },
            orch,
            jval::asLambda(jval::get_VisionWorkerConfig_validator())
        ));
        srv.Patch("/api/local/pipelines/([^/]+.json)",makeHandler_local_json_PATCH(
            "hardware",
            [](const httplib::Request& req){ return req.matches[1].str(); },
            orch,
            jval::asLambda(jval::get_VisionWorkerConfig_validator())
        ));
        srv.Delete("/api/local/pipelines/([^/]+.json)",makeHandler_local_json_DELETE(
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
        srv.Get("/api/resources/fields/([^/]+.json)",makeHandler_resource_json_GET(
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

