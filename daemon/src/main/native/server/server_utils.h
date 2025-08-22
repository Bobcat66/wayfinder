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

#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <optional>
#include <vector>
#include <type_traits>
#include <httplib.h>
#include <format>

namespace wfsrv {

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


    std::optional<std::string> getScriptsPath();

    int invoke_shell(const char* cmd, std::ostream& out_capture = std::cout);

    template <typename... Args>
    inline int invoke_wfcfg_cap(std::ostream& out_capture, Args&&... args) {
        auto scriptPathOpt = getScriptsPath();
        if (!scriptPathOpt) return -1;

        std::ostringstream cmd_oss;
        cmd_oss << scriptPathOpt.value() << "/wfcfg.py";
        ((cmd_oss << " " << std::quoted(args)), ...);

        return invoke_shell(cmd_oss.str().c_str());
    }

    template <typename... Args>
    inline int invoke_wfcfg(Args&&... args) {
        return invoke_wfcfg_cap(std::cout,std::forward<Args>(args)...);
    }

    std::string join(const std::vector<std::string>& vec, const std::string& sep);

     // For when status is known at compile time
    template <int Status, typename... Args>
    inline std::string getErrorResponse(std::string_view fmt,Args&&... args) {
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
    inline std::string getErrorResponse(int status, std::string_view fmt,Args&&... args) {
        return std::format(
            "{{\"status\":{},\"error\":\"{}\",\"message\":\"{}\"}}",
            status,
            getStatusMsg(status),
            std::vformat(fmt,std::make_format_args(args...))
        );
    }

    inline void setContent(httplib::Response& res, const std::string& content) {
        res.set_content(
            content,
            "application/json"
        );
    }

    
}