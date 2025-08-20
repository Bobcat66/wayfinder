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
            auto spathstropt = wf::env::getvar("WF_PATH");
            if (!pathstropt) {
                badpath = true;
                return std::nullopt;
            }
            if (!fs::exists(fs::path(pathstropt.value()))) {
                badpath = true;
                return std::nullopt;
            }
            spath = pathstropt.value();
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
        ((cmd_oss << " " << args), ...);

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
                throw "Unexpected HTTP status code!";
        }
    }

    template <typename... Args>
    std::string getErrorResponse(int status,std::string_view fmt,Args&&... args) {
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

    auto makeHandler_env_team_GET(const wf::WFSystemConfig& sysconf) {
        return [&sysconf](const httplib::Request& req, httplib::Response& res){
            setContent(
                res, std::format("{}", sysconf.team)
            );
        };
    }

    auto makeHandler_env_team_PUT() {
        return [](const httplib::Request& req, httplib::Response& res){
            if (!isDigitStr(req.body)) {
                res.status = 422;
                setContent(
                    res, getErrorResponse(422,"Content is not a numeric string")
                );
                return;
            }
            auto invoke_res = invoke_wfcfg("putenv","WF_TEAM",req.body);
            if (invoke_res == 0) {
                res.status = 204;
            } else {
                res.status = 500;
                setContent(
                    res, getErrorResponse(500,"Error while invoking wfcfg subprocess: {}",invoke_res)
                );
            }
        };
    }

    auto makeHandler_env_devname_GET(const wf::WFSystemConfig& sysconf) {
        return [&sysconf](const httplib::Request& req, httplib::Response& res){
            setContent(
                res, sysconf.device_name
            );
        }
    }

    auto makeHandler_env_devname_PUT() {
        return [](const httplib::Request& req, httplib::Response& res){
            if (!isAlnumStr(req.body)) {
                res.status = 422;
                setContent(
                    res, getErrorResponse(422,"Content is not an alphanumeric string")
                );
                return;
            }
            auto invoke_res = invoke_wfcfg("putenv","WF_DEVICE_NAME",req.body);
            if (invoke_res == 0) {
                res.status = 204;
            } else {
                res.status = 500;
                setContent(
                    res, getErrorResponse(500,"Error while invoking wfcfg subprocess: {}",invoke_res)
                );
            }
        };
    }

    // If the return string is present, the operation failed and the server will return error 500
    using genericPutConsumer = std::function<std::optional<std::string>>(const std::string&)>;

    auto makeHandler_generic_PUT(
        genericPutConsumer consumer,
        bool (*validator)(const std::string&),
        std::string errmsg422 = ""
    ){
        return [consumer,validator,errmsg422](const httplib::Request& req, httplib::Response& res){
            if (!validator(req.body)) {
                res.status = 422;
                setContent(res, getErrorResponse(422,errmsg422));
                return;
            }
            auto cerrmsg = consumer(req.body);
            if (cerrmsg) {
                res.status = 500;
                setContent(res, getErrorResponse(500,cerrmsg.value()));
            } else {
                res.status = 204
            }
        }
    }

    auto makeHandler_generic_PUT(
        genericPutConsumer consumer,
        std::function<bool(const std::string&)> validator
        std::string errmsg422 = ""
    ){
        return [consumer,validator,errmsg422](const httplib::Request& req, httplib::Response& res){
            if (!validator(req.body)) {
                res.status = 422;
                setContent(res, getErrorResponse(422,errmsg422));
                return;
            }
            auto cerrmsg = consumer(req.body);
            if (cerrmsg) {
                res.status = 500;
                setContent(res, getErrorResponse(500,cerrmsg.value()));
            } else {
                res.status = 204
            }
        }
    }

    auto makeHandler_generic_GET(std::function<std::string(void)> resourceGetter) {
        return [resourceGetter](const httplib::Request& req, httplib::Response& res){
            setContent(
                res, resourceGetter()
            );
        }
    }
}

namespace wfsrv {
    HTTPServer::HTTPServer(wf::WFOrchestrator& orchestrator)
    : orchestrator_(orchestrator){
        srv_.Get("/api/env/team",impl::makeHandler_generic_GET(
            [](){return std::format("{}",orchestrator_.getSystemConfig().team);}
        ));
        srv_.Put("/api/env/team",impl::makeHandler_generic_PUT(
            [](const std::string& content){
                auto res = impl::invoke_wfcfg("putenv","WF_TEAM",content);
                if (res == 0) return std::nullopt;
                return std::format("Error while invoking wfcfg subprocess: {}",res);
            },
            impl::isDigitStr,
            "Content is not a numeric string"
        ));
        srv_.Get("/api/env/devname",impl::makeHandler_generic_GET(
            [](){return orchestrator_.getSystemConfig().device_name;}
        ));
        srv_.Put("/api/env/devname",impl::makeHandler_env_devname_PUT());
    }
}

