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

#include "server/server_utils.h"

namespace wfsrv {
    inline auto makeHandler_env_stored_GET(std::string key) {
        return [key](const httplib::Request& req, httplib::Response& res){
            static const std::regex envVarPattern(R"(^[A-Z_]+=(.*))");
            std::ostringstream wfcfg_out;
            int wfcfg_res = invoke_wfcfg_cap(wfcfg_out,"getenv",key);
            // TODO: Dynamic failure codes
            if (wfcfg_res != 0) {
                res.status = 500;
                setContent(res, getErrorResponse<500>("Error while invoking wfcfg subprocess: {}", wfcfg_res));
                return;
            }
            // Read first line from ostringstream. When there are multiple key value pairs of the same key, the one which appears the earliest gets priority.
            // Although there theoretically should never be multiple pairs with the same key
            std::istringstream iss(wfcfg_out.str());
            std::string firstline;
            if (!std::getline(iss,firstline)) {
                // there are zero lines in the output, wfcfg did not print anything to stdout
                res.status = 404;
                setContent(res, getErrorResponse<404>("Key {} not found in env file", key));
                return;
            }

            std::smatch match;
            if (std::regex_match(firstline, match, envVarPattern)) {
                res.status = 200;
                setContent(res, match[1]);
                return;
            }
            res.status = 400;
            setContent(res, getErrorResponse<400>("Key-value pair not recognized in wfcfg output"));
        };
    }

}