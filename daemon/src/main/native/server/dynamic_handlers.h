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

#include "wfcore/common/status.h"
#include "wfcore/configuration/ResourceManager.h"
#include "server/server_utils.h"

namespace wfsrv {

    // A getter to dynamically get JSON files
    template <wf::WFResult<wf::JSON> (wf::ResourceManager::*JSONLoader)(const std::string&,const std::string&) const, typename FilenameGetter>
    inline auto makeHandler_dynamic_file_GET(std::string subdir, FilenameGetter filenameGetter, wf::WFOrchestrator& orch) {
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
    inline auto makeHandler_local_file_GET(std::string subdir, FilenameGetter filenameGetter, wf::WFOrchestrator& orch) {
        return makeHandler_dynamic_file_GET<&wf::ResourceManager::loadLocalJSON>(subdir,filenameGetter,orch);
    }

    // FilenameGetter should take a const http Request and return a string
    template <typename FilenameGetter>
    inline auto makeHandler_resource_file_GET(std::string subdir, FilenameGetter filenameGetter, wf::WFOrchestrator& orch) {
        return makeHandler_dynamic_file_GET<&wf::ResourceManager::loadResourceJSON>(subdir,filenameGetter,orch);
    }

    // FilenameGetter should take a const http Request and return a string
    // JSONValidatorCallable should be a callable which takes a const JSON& object and returns a JVResult or WFResult (JVResult is preferred)
    template <typename JSONValidatorCallable, typename FilenameGetter>
    inline auto makeHandler_local_file_PUT(std::string subdir, FilenameGetter filenameGetter, wf::WFOrchestrator& orch, JSONValidatorCallable validator) {
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
    inline auto makeHandler_local_file_PATCH(std::string subdir, FilenameGetter filenameGetter, wf::WFOrchestrator& orch, JSONValidatorCallable validator) {
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
    inline auto makeHandler_local_file_PUT(std::string subdir, FilenameGetter filenameGetter, wf::WFOrchestrator& orch) {
        return makeHandler_local_file_PUT(subdir,filenameGetter,orch,jval::asLambda(jval::getNullValidator()));
    }

    template <typename FilenameGetter>
    inline auto makeHandler_local_file_DELETE(std::string subdir, FilenameGetter filenameGetter, wf::WFOrchestrator& orch) {
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
    inline auto makeHandler_enum_subdir(std::string subdir, wf::WFOrchestrator& orch) {
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

    inline auto makeHandler_enum_local_subdir(std::string subdir, wf::WFOrchestrator& orch) {
        return makeHandler_enum_subdir<&wf::ResourceManager::enumerateLocalSubdir>(subdir,orch);
    }

    inline auto makeHandler_enum_resource_subdir(std::string subdir, wf::WFOrchestrator& orch) {
        return makeHandler_enum_subdir<&wf::ResourceManager::enumerateResourceSubdir>(subdir,orch);
    }
}