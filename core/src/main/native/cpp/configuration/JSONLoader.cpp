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

#include "wfcore/configuration/JSONLoader.h"
#include "wfcore/common/logging.h"
#include "wfcore/common/wfexcept.h"
#include <fstream>
#include <unordered_map>

namespace fs = std::filesystem;

namespace wf {

    using enum WFStatus;

    JSONLoader::JSONLoader(
        std::filesystem::path resourceDir,
        std::filesystem::path localDir
    ) 
    : resourceDir_(resourceDir)
    , localDir_(localDir) {
        if (!fs::is_directory(resourceDir_)){
            auto rdirstr = resourceDir_.string();
            throw bad_resource_dir("'{}' is not a directory",rdirstr);
        }
        
        if (!fs::is_directory(localDir_)){
            auto ldirstr = localDir_.string();
            throw bad_local_dir("'{}' is not a directory",ldirstr);
        }
    }

    WFResult<JSON> JSONLoader::loadResourceJSON(const std::string& subdirName,const std::string& filename) const {
        JSON jobject;

        WF_DEBUGLOG(globalLogger(), "Searching for resource subdir '{}'",subdirName);
        auto it = resourceSubdirs.find(subdirName);
        if (it == resourceSubdirs.end()) 
            return WFResult<JSON>::failure(CONFIG_BAD_SUBDIR,"'{}' is not recognized as a valid resource subdir",subdirName);
        
        fs::path subdirPath = resourceDir_ / it->second;
        if (!fs::exists(subdirPath) || !fs::is_directory(subdirPath)) 
            return WFResult<JSON>::failure(CONFIG_BAD_SUBDIR,"'{}' is not a valid directory",subdirPath.string());

        fs::path resourcePath = subdirPath / filename;
        WF_DEBUGLOG(globalLogger(), "Searching for resource file '{}'",resourcePath.string());
        if (!fs::exists(resourcePath))
            return WFResult<JSON>::failure(FILE_NOT_FOUND,"File '{}' does not exist",resourcePath.string());

        WF_DEBUGLOG(globalLogger(), "Opening file '{}'", resourcePath.string());
        std::ifstream file(resourcePath);
        if (!file.is_open())
            return WFResult<JSON>::failure(FILE_NOT_OPENED,"Failed to open file '{}'", resourcePath.string());
        
        try {
            file >> jobject;
            return WFResult<JSON>::success(std::move(jobject));
        } catch (const JSON::parse_error& e) {
            return WFResult<JSON>::failure(
                JSON_PARSE,
                "JSON Parse error {} at byte {}: {}",
                e.id, e.byte, e.what()
            );
        } catch (const std::exception& e) {
            return WFResult<JSON>::failure(UNKNOWN,"Unknown error while parsing JSON: {}",e.what());
        }
        
    }

    WFResult<JSON> JSONLoader::loadLocalJSON(const std::string& subdirName,const std::string& filename) const {
        JSON jobject;

        WF_DEBUGLOG(globalLogger(), "Searching for local subdir '{}'",subdirName);
        auto it = localSubdirs.find(subdirName);
        if (it == localSubdirs.end()) 
            return WFResult<JSON>::failure(CONFIG_SUBDIR_NOT_FOUND,"'{}' is not recognized as a valid local subdir",subdirName);

        fs::path subdirPath = localDir_ / it->second;
        if (!fs::exists(subdirPath) || !fs::is_directory(subdirPath)) 
            return WFResult<JSON>::failure(CONFIG_BAD_SUBDIR,"'{}' is not a valid directory",subdirPath.string());
        

        fs::path localPath = subdirPath / filename;
        WF_DEBUGLOG(globalLogger(), "Searching for local file '{}'",localPath.string());
        if (!fs::exists(localPath))
            return WFResult<JSON>::failure(FILE_NOT_FOUND,"File '{}' does not exist",localPath.string());

        WF_DEBUGLOG(globalLogger(), "Opening file '{}'", localPath.string());
        std::ifstream file(localPath);
        if (!file.is_open())
            return WFResult<JSON>::failure(FILE_NOT_OPENED,"Failed to open file '{}'", localPath.string());
        
        try {
            file >> jobject;
            return WFResult<JSON>::success(std::move(jobject));
        } catch (const JSON::parse_error& e) {
            return WFResult<JSON>::failure(
                JSON_PARSE,
                "JSON Parse error {} at byte {}: {}",
                e.id, e.byte, e.what()
            );
        } catch (const std::exception& e) {
            return WFResult<JSON>::failure(UNKNOWN,"Unknown error while parsing JSON: {}",e.what());
        }
        
    }

    // This will completely overrwrite the file if it already exists, use with caution
    WFStatusResult JSONLoader::storeLocalJSON(const std::string& subdirName, const std::string& filename, const JSON& jobject) const {
        WF_DEBUGLOG(globalLogger(),"Searching for local subdir {}",subdirName);
        auto it = localSubdirs.find(subdirName);
        if (it == localSubdirs.end()) 
            return WFStatusResult::failure(CONFIG_SUBDIR_NOT_FOUND,"'{}' is not recognized as a valid local subdir",subdirName);

        fs::path subdirPath = localDir_ / it->second;
        if (!fs::exists(subdirPath) || !fs::is_directory(subdirPath)) 
            return WFStatusResult::failure(CONFIG_BAD_SUBDIR,"'{}' is not a valid directory",subdirPath.string());

        fs::path localPath =  subdirPath / filename;
        WF_DEBUGLOG(globalLogger(),"Opening local file {}",localPath.string());
        std::ofstream file(localPath);
        if (!file.is_open()) return WFStatusResult::failure(FILE_NOT_OPENED,"Failed to open file '{}'",localPath.string());
        
        WF_DEBUGLOG(globalLogger(),"Writing to local file {}",localPath.string());
        // Pretty-prints the JSON object with indentation of 4 for human readability
        file << jobject.dump(4);
        
        return file.good()
            ? WFStatusResult::success()
            : WFStatusResult::failure(UNKNOWN,"File stream error");
    }

    WFResult<std::vector<std::string>> JSONLoader::enumerateResourceSubdir(const std::string& subdirName) const {
        WF_DEBUGLOG(globalLogger(),"Enumerating resource subdir {}",subdirName);
        auto it = resourceSubdirs.find(subdirName);
        if (it == resourceSubdirs.end())
            return WFResult<std::vector<std::string>>::failure(CONFIG_SUBDIR_NOT_FOUND,"'{}' is not recognized as a valid resource subdir",subdirName);
        
        fs::path subdirPath = resourceDir_ / it->second;
        if (!fs::exists(subdirPath) || !fs::is_directory(subdirPath)) 
            return WFResult<std::vector<std::string>>::failure(CONFIG_BAD_SUBDIR,"'{}' is not a valid directory",subdirPath.string());

        std::vector<std::string> subdirFiles;
        for (const auto& entry : fs::directory_iterator(subdirPath)) {
            if (entry.is_regular_file()) {
                WF_DEBUGLOG(globalLogger(),"Found file {} in subdir {}",entry.path().string(),subdirName);
                subdirFiles.push_back(entry.path().filename().string());
            }
        }

        return WFResult<std::vector<std::string>>::success(std::move(subdirFiles));
    }

    WFResult<std::vector<std::string>> JSONLoader::enumerateLocalSubdir(const std::string& subdirName) const {
        WF_DEBUGLOG(globalLogger(),"Enumerating local subdir {}",subdirName);
        auto it = localSubdirs.find(subdirName);
        if (it == localSubdirs.end())
            return WFResult<std::vector<std::string>>::failure(CONFIG_SUBDIR_NOT_FOUND,"'{}' is not recognized as a valid local subdir",subdirName);
        
        fs::path subdirPath = localDir_ / it->second;
        if (!fs::exists(subdirPath) || !fs::is_directory(subdirPath)) 
            return WFResult<std::vector<std::string>>::failure(CONFIG_BAD_SUBDIR,"'{}' is not a valid directory",subdirPath.string());

        std::vector<std::string> subdirFiles;
        for (const auto& entry : fs::directory_iterator(subdirPath)) {
            if (entry.is_regular_file()) {
                WF_DEBUGLOG(globalLogger(),"Found file {} in subdir {}",entry.path().string(),subdirName);
                subdirFiles.push_back(entry.path().filename().string());
            }
        }

        return WFResult<std::vector<std::string>>::success(std::move(subdirFiles));
    }

    std::vector<std::string> JSONLoader::enumerateResourceSubdirs() const {
        std::vector<std::string> subdirs;
        for (const auto& [key,value] : resourceSubdirs) {
            subdirs.emplace_back(key);
        }
        return subdirs;
    }
    std::vector<std::string> JSONLoader::enumerateLocalSubdirs() const {
        std::vector<std::string> subdirs;
        for (const auto& [key,value] : localSubdirs) {
            subdirs.emplace_back(key);
        }
        return subdirs;
    }
    bool JSONLoader::resourceSubdirExists(const std::string& name) const {
        auto it = resourceSubdirs.find(name);
        return it != resourceSubdirs.end();
    }
    bool JSONLoader::localSubdirExists(const std::string& name) const {
        auto it = localSubdirs.find(name);
        return it != localSubdirs.end();
    }

    WFStatusResult JSONLoader::assignLocalSubdir(const std::string& subdirName,const std::filesystem::path& subdirRelpath) {
        WF_DEBUGLOG(globalLogger(),"Assigning local subdir {} to relpath {}",subdirName,subdirRelpath.string());

        // Validate subdir path
        fs::path fullpath = localDir_ / subdirRelpath;
        if (!fs::exists(fullpath) || !fs::is_directory(fullpath))
            return WFStatusResult::failure(CONFIG_BAD_SUBDIR,"'{}' is not a valid directory",fullpath.string());
        
        localSubdirs[subdirName] = subdirRelpath;
        return WFStatusResult::success();
    }

    WFStatusResult JSONLoader::assignResourceSubdir(const std::string& subdirName,const std::filesystem::path& subdirRelpath) {
        WF_DEBUGLOG(globalLogger(),"Assigning resource subdir {} to relpath {}",subdirName,subdirRelpath.string());

        // Validate subdir path
        fs::path fullpath = resourceDir_ / subdirRelpath;
        if (!fs::exists(fullpath) || !fs::is_directory(fullpath))
            return WFStatusResult::failure(CONFIG_BAD_SUBDIR,"'{}' is not a valid directory",fullpath.string());
        
        resourceSubdirs[subdirName] = subdirRelpath;
        return WFStatusResult::success();
    }






}
