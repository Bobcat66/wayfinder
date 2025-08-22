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

#pragma once

#include "wfcore/common/status/StatusfulObject.h"
#include "wfcore/common/status/LoggedStatusfulObject.h"
#include "wfcore/common/status/ConcurrentStatusfulObject.h"
#include "wfcore/common/status/ConcurrentLoggedStatusfulObject.h"
#include "wfcore/common/status/StatusfulResult.h"

#include <cstdint>

namespace wf {

    constexpr std::uint32_t CATEGORY_MASK = 0xfff00000;

    // The first three digits specify the context of the error code, 
    // and the remaining five hex digits specify the specific error code
    // As an example, every status code with the format 0x001XXXXX comes from a pipeline
    // The status codes 0x00000001 to 0x000fffff are reserved for uncategorized status codes
    // 0x00000000 is reserved as the sole "everything's fine" status, and has its own special category

    // TODO: Refactor everything to use WFStatus
    enum class WFStatus : std::uint32_t {
        OK =                        0x00000000,
        UNCATEGORIZED_BASE =        0x00000000, // Ts is lowkey scuffed, but whatever
        BAD_ARGUMENT =              0x00000001,
        BAD_ALLOC =                 0x00000002,
        OUT_OF_BOUNDS =             0x00000003,
        BAD_ASSERT =                0x00000004,
        FILE_NOT_OPENED =           0x00000005,
        FILE_NOT_FOUND =            0x00000006,
        NOT_IMPLEMENTED =           0x00000007,
        BAD_ACQUIRE =               0x00000008,
        UNKNOWN =                   0x000fffff,

        PIPELINE_BASE =             0x00100000,
        PIPELINE_BAD_FRAME =        0x00100001,
        PIPELINE_BAD_CONFIG =       0x00100002,
        PIPELINE_UNKNOWN =          0x001fffff,

        APRILTAG_BASE =             0x00200000,
        APRILTAG_BAD_FAMILY =       0x00200001,
        APRILTAG_BAD_POSE =         0x00200002,
        APRILTAG_NO_INTRINSICS =    0x00200003,
        APRILTAG_UNKNOWN =          0x002fffff,

        INFERENCE_BASE =            0x00300000,
        INFERENCE_BAD_MODEL =       0x00300001,
        INFERENCE_BAD_INPUT =       0x00300002,
        INFERENCE_BAD_PASS =        0x00300003,
        INFERENCE_UNKNOWN =         0x003fffff,

        HARDWARE_BASE =             0x00400000,
        HARDWARE_BAD_CONTROL =      0x00400001,
        HARDWARE_BAD_FORMAT =       0x00400002,
        HARDWARE_CSCORE =           0x00400003,
        HARDWARE_DISCONNECT =       0x00400004,
        HARDWARE_BAD_BACKEND =      0x00400005,
        HARDWARE_BAD_CAMERA =       0x00400006,
        HARDWARE_NO_CALIB =         0x00400007,
        HARDWARE_DISABLED =         0x00400008,
        HARDWARE_CONNECTING =       0x00400009,
        HARDWARE_UNKNOWN =          0x004fffff,

        NETWORK_BASE =              0x00500000,
        NETWORK_UNKNOWN =           0x005fffff,

        JSON_BASE =                 0x00600000,
        JSON_PROPERTY_NOT_FOUND =   0x00600001,
        JSON_INVALID_TYPE =         0x00600002,
        JSON_SCHEMA_VIOLATION =     0x00600003,
        JSON_PARSE =                0x00600004,
        JSON_UNKNOWN =              0x006fffff,

        GRAPH_BASE =                0x00700000,
        GRAPH_UNKNOWN =             0x007fffff,

        VIDEO_BASE =                0x00800000,
        VIDEO_UNKNOWN =             0x008fffff,

        CONFIG_BASE =               0x00900000,
        CONFIG_BAD_SUBDIR =         0x00900001,
        CONFIG_BAD_LOCALDIR =       0x00900002,
        CONFIG_BAD_RESOURCEDIR =    0x00900003,
        CONFIG_SUBDIR_NOT_FOUND =   0x00900004,
        CONFIG_UNKNOWN =            0x009fffff,

        SERDE_BASE =                0x00a00000,
        SERDE_WIPS_OOM =            0x00a00001, // 0x00a00001 to 0x00a000ff are reserved for WIPS status codes
        SERDE_WIPS_BOUNDS_ERROR =   0x00a00002,
        SERDE_WIPS_OVERFLOW =       0x00a00003,
        SERDE_WIPS_BAD_ASSERT =     0x00a00004,
        SERDE_WIPS_UNKNOWN =        0x00a000ff,
        SERDE_UNKNOWN =             0x00afffff,

        ENV_BASE =                  0x00b00000,
        ENV_BAD_ENVIRONMENT =       0x00b00001,
        ENV_BAD_KEY =               0x00b00002,
        ENV_BAD_FORMAT =            0x00b00003,
        ENV_BAD_RANGE =             0x00b00004,
        ENV_UNKNOWN =               0x00bfffff
    };

    inline constexpr const char* wfstatus_name(WFStatus status) {
        switch (status) {
            case WFStatus::OK:                          return "OK";
            case WFStatus::BAD_ARGUMENT:                return "BAD_ARGUMENT";
            case WFStatus::BAD_ALLOC:                   return "BAD_ALLOC";
            case WFStatus::OUT_OF_BOUNDS:               return "OUT_OF_BOUNDS";
            case WFStatus::BAD_ASSERT:                  return "BAD_ASSERT";
            case WFStatus::FILE_NOT_OPENED:             return "FILE_NOT_OPENED";
            case WFStatus::FILE_NOT_FOUND:              return "FILE_NOT_FOUND";
            case WFStatus::NOT_IMPLEMENTED:             return "NOT_IMPLEMENTED";
            case WFStatus::BAD_ACQUIRE:                 return "BAD_ACQUIRE";
            case WFStatus::UNKNOWN:                     return "UNKNOWN";

            case WFStatus::PIPELINE_BASE:               return "PIPELINE_BASE";
            case WFStatus::PIPELINE_BAD_FRAME:          return "PIPELINE_BAD_FRAME";
            case WFStatus::PIPELINE_BAD_CONFIG:         return "PIPELINE_BAD_CONFIG";
            case WFStatus::PIPELINE_UNKNOWN:            return "PIPELINE_UNKNOWN";

            case WFStatus::APRILTAG_BASE:               return "APRILTAG_BASE";
            case WFStatus::APRILTAG_BAD_FAMILY:         return "APRILTAG_BAD_FAMILY";
            case WFStatus::APRILTAG_BAD_POSE:           return "APRILTAG_BAD_POSE";
            case WFStatus::APRILTAG_NO_INTRINSICS:      return "APRILTAG_NO_INTRINSICS";
            case WFStatus::APRILTAG_UNKNOWN:            return "APRILTAG_UNKNOWN";

            case WFStatus::INFERENCE_BASE:              return "INFERENCE_BASE";
            case WFStatus::INFERENCE_BAD_MODEL:         return "INFERENCE_BAD_MODEL";
            case WFStatus::INFERENCE_BAD_INPUT:         return "INFERENCE_BAD_INPUT";
            case WFStatus::INFERENCE_BAD_PASS:          return "INFERENCE_BAD_PASS";
            case WFStatus::INFERENCE_UNKNOWN:           return "INFERENCE_UNKNOWN";

            case WFStatus::HARDWARE_BASE:               return "HARDWARE_BASE";
            case WFStatus::HARDWARE_BAD_CONTROL:        return "HARDWARE_BAD_CONTROL";
            case WFStatus::HARDWARE_BAD_FORMAT:         return "HARDWARE_BAD_FORMAT";
            case WFStatus::HARDWARE_CSCORE:             return "HARDWARE_CSCORE";
            case WFStatus::HARDWARE_DISCONNECT:         return "HARDWARE_DISCONNECT";
            case WFStatus::HARDWARE_BAD_BACKEND:        return "HARDWARE_BAD_BACKEND";
            case WFStatus::HARDWARE_BAD_CAMERA:         return "HARDWARE_BAD_CAMERA";
            case WFStatus::HARDWARE_NO_CALIB:           return "HARDWARE_NO_CALIB";
            case WFStatus::HARDWARE_DISABLED:           return "HARDWARE_DISABLED";
            case WFStatus::HARDWARE_CONNECTING:         return "HARDWARE_CONNECTING";
            case WFStatus::HARDWARE_UNKNOWN:            return "HARDWARE_UNKNOWN";

            case WFStatus::NETWORK_BASE:                return "NETWORK_BASE";
            case WFStatus::NETWORK_UNKNOWN:             return "NETWORK_UNKNOWN";

            case WFStatus::JSON_BASE:                   return "JSON_BASE";
            case WFStatus::JSON_PROPERTY_NOT_FOUND:     return "JSON_PROPERTY_NOT_FOUND";
            case WFStatus::JSON_INVALID_TYPE:           return "JSON_INVALID_TYPE";
            case WFStatus::JSON_SCHEMA_VIOLATION:       return "JSON_SCHEMA_VIOLATION";
            case WFStatus::JSON_PARSE:                  return "JSON_PARSE";
            case WFStatus::JSON_UNKNOWN:                return "JSON_UNKNOWN";

            case WFStatus::GRAPH_BASE:                  return "GRAPH_BASE";
            case WFStatus::GRAPH_UNKNOWN:               return "GRAPH_UNKNOWN";

            case WFStatus::VIDEO_BASE:                  return "VIDEO_BASE";
            case WFStatus::VIDEO_UNKNOWN:               return "VIDEO_UNKNOWN";

            case WFStatus::CONFIG_BASE:                 return "CONFIG_BASE";
            case WFStatus::CONFIG_BAD_SUBDIR:           return "CONFIG_BAD_SUBDIR";
            case WFStatus::CONFIG_BAD_LOCALDIR:         return "CONFIG_BAD_LOCALDIR";
            case WFStatus::CONFIG_BAD_RESOURCEDIR:      return "CONFIG_BAD_RESOURCEDIR";
            case WFStatus::CONFIG_SUBDIR_NOT_FOUND:     return "CONFIG_SUBDIR_NOT_FOUND";
            case WFStatus::CONFIG_UNKNOWN:              return "CONFIG_UNKNOWN";

            case WFStatus::SERDE_BASE:                  return "SERDE_BASE";
            case WFStatus::SERDE_WIPS_OOM:              return "SERDE_WIPS_OOM";
            case WFStatus::SERDE_WIPS_BOUNDS_ERROR:     return "SERDE_WIPS_BOUNDS_ERROR";
            case WFStatus::SERDE_WIPS_OVERFLOW:         return "SERDE_WIPS_OVERFLOW";
            case WFStatus::SERDE_WIPS_BAD_ASSERT:       return "SERDE_WIPS_BAD_ASSERT";
            case WFStatus::SERDE_WIPS_UNKNOWN:          return "SERDE_WIPS_UNKNOWN";
            case WFStatus::SERDE_UNKNOWN:               return "SERDE_UNKNOWN";

            case WFStatus::ENV_BASE:                    return "ENV_BASE";
            case WFStatus::ENV_BAD_ENVIRONMENT:         return "ENV_BAD_ENVIRONMENT";
            case WFStatus::ENV_BAD_KEY:                 return "ENV_BAD_KEY";
            case WFStatus::ENV_BAD_FORMAT:              return "ENV_BAD_FORMAT";
            case WFStatus::ENV_BAD_RANGE:               return "ENV_BAD_RANGE";
            case WFStatus::ENV_UNKNOWN:                 return "ENV_UNKNOWN";

            default:                                    return "UNRECOGNIZED";
        }
    }

    inline constexpr std::string_view wfstatus_name_view(WFStatus status) {
        return wfstatus_name(status);
    }

    // Returns the category base of an error code
    inline constexpr WFStatus wfstatus_category_base(WFStatus status) {
        return static_cast<WFStatus>(static_cast<uint32_t>(status) & CATEGORY_MASK); 
    }

    inline constexpr std::string_view wfstatus_category_name(WFStatus status) {
        // WFStatus::OK is a special case that needs to be handled separately
        if (status == WFStatus::OK)                 return "Nominal";
        switch (wfstatus_category_base(status)) {
            case WFStatus::UNCATEGORIZED_BASE:      return "Uncategorized";
            case WFStatus::PIPELINE_BASE:           return "Pipeline";
            case WFStatus::APRILTAG_BASE:           return "Apriltag";
            case WFStatus::INFERENCE_BASE:          return "Inference";
            case WFStatus::HARDWARE_BASE:           return "Hardware";
            case WFStatus::NETWORK_BASE:            return "Network";
            case WFStatus::JSON_BASE:               return "JSON";
            case WFStatus::GRAPH_BASE:              return "Graph";
            case WFStatus::VIDEO_BASE:              return "Video";
            case WFStatus::CONFIG_BASE:             return "Config";
            case WFStatus::SERDE_BASE:              return "Serde";
            case WFStatus::ENV_BASE:                return "Environment";
            default:                                return "Unknown";
        }
    }

    using WFStatusfulObject = StatusfulObject<WFStatus,WFStatus::OK,wfstatus_name>;
    using WFConcurrentStatusfulObject = ConcurrentStatusfulObject<WFStatus,WFStatus::OK,wfstatus_name>;
    using WFLoggedStatusfulObject = LoggedStatusfulObject<WFStatus,WFStatus::OK,wfstatus_name>;
    using WFConcurrentLoggedStatusfulObject = ConcurrentLoggedStatusfulObject<WFStatus,WFStatus::OK,wfstatus_name>;

    template <typename T>
    using WFResult = StatusfulResult<T,WFStatus,WFStatus::OK,wfstatus_name>;

    using WFStatusResult = StatusResult<WFStatus,WFStatus::OK,wfstatus_name>;
}