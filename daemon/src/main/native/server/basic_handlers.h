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
    // ConsumerCallable should be a callable which takes a const std::string& and returns an std::optional<std::string>>
    // If the return string is present, the operation failed and the server will return error 500
    // JSONValidatorCallable should be a callable which takes a JSON object and returns a JVResult
    template <typename ConsumerCallable, typename JSONValidatorCallable>
    inline auto makeHandler_generic_PUT(
        ConsumerCallable consumer,
        JSONValidatorCallable validator
    ){
        return [consumer,validator](const httplib::Request& req, httplib::Response& res){
            wf::JSON jobject;
            try {
                jobject = wf::JSON::parse(req.body);
            } catch (const wf::JSON::parse_error& e) {
                res.status = 400;
                setContent(res, getErrorResponse<400>(e.what()));
                return;
            }
            auto valres = validator(jobject);
            if (!valres) {
                res.status = 422;
                setContent(res, getErrorResponse<422>(valres.what()));
                return;
            }
            std::optional<std::string> cerrmsg = consumer(jobject.dump());
            if (cerrmsg) {
                res.status = 500;
                setContent(res, getErrorResponse<500>(cerrmsg.value()));
            } else {
                res.status = 204;
            }
        };
    }

    template <typename ConsumerCallable>
    inline auto makeHandler_int_PUT(ConsumerCallable consumer) {
        return makeHandler_generic_PUT(consumer,jval::asLambda(jval::getPrimitiveValidator<int>()));
    }

    template <typename ConsumerCallable>
    inline auto makeHandler_double_PUT(ConsumerCallable consumer) {
        return makeHandler_generic_PUT(consumer,jval::asLambda(jval::getPrimitiveValidator<double>()));
    }

    template <typename ConsumerCallable>
    inline auto makeHandler_bool_PUT(ConsumerCallable consumer) {
        return makeHandler_generic_PUT(consumer,jval::asLambda(jval::getPrimitiveValidator<bool>()));
    }

    template <typename ConsumerCallable>
    inline auto makeHandler_string_PUT(ConsumerCallable consumer) {
        return makeHandler_generic_PUT(consumer,jval::asLambda(jval::getPrimitiveValidator<std::string>()));
    }

    // GetterCallable should be a callable taking void args and returning a string-like object
    // FormatCallable should be a callable taking a string and returning a string formatted as JSON
    template <typename GetterCallable, typename FormatCallable>
    inline auto makeHandler_generic_GET(GetterCallable resourceGetter, FormatCallable formatter) {
        return [resourceGetter,formatter](const httplib::Request& req, httplib::Response& res){
            res.status = 200;
            setContent(res, formatter(resourceGetter()));
        };
    }

    // the raw string representations of ints bools and doubles are identical to their JSON representations
    template <typename GetterCallable>
    inline auto makeHandler_int_GET(GetterCallable resourceGetter) {
        return makeHandler_generic_GET(resourceGetter,[](const std::string& msg){return msg;});
    }

    template <typename GetterCallable>
    inline auto makeHandler_bool_GET(GetterCallable resourceGetter) {
        return makeHandler_generic_GET(resourceGetter,[](const std::string& msg){return msg;});
    }

    template <typename GetterCallable>
    inline auto makeHandler_double_GET(GetterCallable resourceGetter) {
        return makeHandler_generic_GET(resourceGetter,[](const std::string& msg){return msg;});
    }

    template <typename GetterCallable>
    inline auto makeHandler_string_GET(GetterCallable resourceGetter) {
        return makeHandler_generic_GET(resourceGetter,[](const std::string& msg){
            return std::format("\"{}\"",msg);
        });
    }

    inline auto makeHandler_OPTIONS(std::vector<std::string> supportedMethods) {
        return [supportedMethods](const httplib::Request& req, httplib::Response& res){
            res.set_header("X-Clacks-Overhead","GNU Terry Pratchett");
            res.set_header("Allow",join(supportedMethods,", "));
            res.status = 204;
        };
    }
}