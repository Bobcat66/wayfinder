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
}

namespace wfsrv {
    HTTPServer::HTTPServer(wf::WFOrchestrator& orchestrator)
    : orchestrator_(orchestrator){

        //api/env/team GET PUT OPTIONS
        srv_.Get("/api/env/team",impl::makeHandler_generic_GET(
            [this](){return std::format("{}",this->orchestrator_.getSystemConfig().team);}
        ));
        srv_.Put("/api/env/team",impl::makeHandler_generic_PUT(
            [](const std::string& content) -> std::optional<std::string> {
                auto res = impl::invoke_wfcfg("putenv","WF_TEAM",content);
                if (res == 0) return std::nullopt;
                return std::format("Error while invoking wfcfg subprocess: {}",res);
            },
            impl::isDigitStr,
            "Content is not a numeric string"
        ));
        srv_.Options("api/env/team",impl::makeHandler_OPTIONS({"GET","PUT","OPTIONS"}));

        //api/env/team GET PUT
        srv_.Get("/api/env/devname",impl::makeHandler_generic_GET(
            [this](){return this->orchestrator_.getSystemConfig().device_name;}
        ));
        srv_.Put("/api/env/devname",impl::makeHandler_generic_PUT(
            [](const std::string& content) -> std::optional<std::string> {
                auto res = impl::invoke_wfcfg("putenv","WF_DEVICE_NAME",content);
                if (res == 0) return std::nullopt;
                return std::format("Error while invoking wfcfg subprocess: {}",res);
            },
            impl::isAlnumStr,
            "Content is not an alphanumeric string"
        ));
        srv_.Options("api/env/slam",impl::makeHandler_OPTIONS({"GET","PUT","OPTIONS"}));

        //api/env/slam GET PUT
        srv_.Get("/api/env/slam",impl::makeHandler_generic_GET(
            [this](){return std::format("{}",this->orchestrator_.getSystemConfig().slam_server);}
        ));
        srv_.Put("/api/env/slam",impl::makeHandler_generic_PUT(
            [](const std::string& content) -> std::optional<std::string> {
                auto res = impl::invoke_wfcfg("putenv","WF_SLAM_SERVER",content);
                if (res == 0) return std::nullopt;
                return std::format("Error while invoking wfcfg subprocess: {}",res);
            },
            impl::isBoolStr,
            "Content is not a boolean string"
        ));
        srv_.Options("api/env/slam",impl::makeHandler_OPTIONS({"GET","PUT","OPTIONS"}));
    }
}

