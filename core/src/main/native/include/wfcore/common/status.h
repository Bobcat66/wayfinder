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

// TODO: Refactor code to use broader status categories
// (Not every StatusfulObject derived class needs its own special type of status)
#pragma once

#include "wfcore/common/status/StatusfulObject.h"
#include "wfcore/common/status/LoggedStatusfulObject.h"
#include "wfcore/common/status/ConcurrentStatusfulObject.h"
#include "wfcore/common/status/ConcurrentLoggedStatusfulObject.h"
#include "wfcore/common/status/StatusfulResult.h"

#include <cstdint>

namespace wf {

    constexpr uint32_t CATEGORY_MASK = 0xFFF00000;

    // The first three digits specify the context of the error code, 
    // and the remaining five hex digits specify the specific error code
    // As an example, every status code with the format 0x001XXXXX comes from a pipeline
    // The status codes 0x00000001 to 0x000FFFFF are reserved for uncategorized status codes
    // 0x00000000 is reserved as the sole "everything's fine" status, and has its own special category

    // TODO: Refactor everything to use WFStatus
    enum class WFStatus : uint32_t {
        OK =                        0x00000000,
        UNCATEGORIZED_BASE =        0x00000000, // Ts is lowkey scuffed, but whatever
        UNKNOWN =                   0x000FFFFF,

        PIPELINE_BASE =             0x00100000,
        PIPELINE_UNKNOWN =          0x001FFFFF,

        APRILTAG_BASE =             0x00200000,
        APRILTAG_UNKNOWN =          0x002FFFFF,

        INFERENCE_BASE =            0x00300000,
        INFERENCE_UNKNOWN =         0x003FFFFF,

        HARDWARE_BASE =             0x00400000,
        HARDWARE_UNKNOWN =          0x004FFFFF,

        NETWORK_BASE =              0x00500000,
        NETWORK_UNKNOWN =           0x005FFFFF,

        JSON_BASE =                 0x00600000,
        JSON_PROPERTY_NOT_FOUND =   0x00600001,
        JSON_INVALID_TYPE =         0x00600002,
        JSON_SCHEMA_VIOLATION =     0x00600003,
        JSON_UNKNOWN =              0x006FFFFF,

        GRAPH_BASE =                0x00700000,
        GRAPH_UNKNOWN =             0x007FFFFF,

        VIDEO_BASE =                0x00800000,
        VIDEO_UNKNOWN =             0x008FFFFF
    };

    constexpr std::string_view wfstatus_name(WFStatus status) {
        switch (status) {
            case WFStatus::OK:                          return "OK";
            case WFStatus::UNKNOWN:                     return "UNKNOWN";

            case WFStatus::PIPELINE_BASE:               return "PIPELINE_BASE";
            case WFStatus::PIPELINE_UNKNOWN:            return "PIPELINE_UNKNOWN";

            case WFStatus::APRILTAG_BASE:               return "APRILTAG_BASE";
            case WFStatus::APRILTAG_UNKNOWN:            return "APRILTAG_UNKNOWN";

            case WFStatus::INFERENCE_BASE:              return "INFERENCE_BASE";
            case WFStatus::INFERENCE_UNKNOWN:           return "INFERENCE_UNKNOWN";

            case WFStatus::HARDWARE_BASE:               return "HARDWARE_BASE";
            case WFStatus::HARDWARE_UNKNOWN:            return "HARDWARE_UNKNOWN";

            case WFStatus::NETWORK_BASE:                return "NETWORK_BASE";
            case WFStatus::NETWORK_UNKNOWN:             return "NETWORK_UNKNOWN";

            case WFStatus::JSON_BASE:                   return "JSON_BASE";
            case WFStatus::JSON_PROPERTY_NOT_FOUND:     return "JSON_PROPERTY_NOT_FOUND";
            case WFStatus::JSON_INVALID_TYPE:           return "JSON_INVALID_TYPE";
            case WFStatus::JSON_SCHEMA_VIOLATION:       return "JSON_SCHEMA_VIOLATION";
            case WFStatus::JSON_UNKNOWN:                return "JSON_UNKNOWN";

            case WFStatus::GRAPH_BASE:                  return "GRAPH_BASE";
            case WFStatus::GRAPH_UNKNOWN:               return "GRAPH_UNKNOWN";

            case WFStatus::VIDEO_BASE:                  return "VIDEO_BASE";
            case WFStatus::VIDEO_UNKNOWN:               return "VIDEO_UNKNOWN";

            default:                                    return "UNRECOGNIZED";
        }
    }

    // Returns the category base of an error code
    constexpr WFStatus wfstatus_catbase(WFStatus status) {
        return static_cast<WFStatus>(static_cast<uint32_t>(status) & CATEGORY_MASK); 
    }

    constexpr std::string_view wfstatus_catname(WFStatus status) {
        // WFStatus::OK is a special case that needs to be handled separately
        if (status == WFStatus::OK)                 return "Nominal";
        switch (wfstatus_catbase(status)) {
            case WFStatus::UNCATEGORIZED_BASE:      return "Uncategorized";
            case WFStatus::PIPELINE_BASE:           return "Pipeline";
            case WFStatus::APRILTAG_BASE:           return "Apriltag";
            case WFStatus::INFERENCE_BASE:          return "Inference";
            case WFStatus::HARDWARE_BASE:           return "Hardware";
            case WFStatus::NETWORK_BASE:            return "Network";
            case WFStatus::JSON_BASE:               return "JSON";
            case WFStatus::GRAPH_BASE:              return "Graph";
            case WFStatus::VIDEO_BASE:              return "Video";
            default:                                return "Unknown";
        }
    }

    using WFStatusfulObject = StatusfulObject<WFStatus,WFStatus::OK>;
    using WFConcurrentStatusfulObject = ConcurrentStatusfulObject<WFStatus,WFStatus::OK>;
    using WFLoggedStatusfulObject = LoggedStatusfulObject<WFStatus,WFStatus::OK>;
    using WFConcurrentLoggedStatusfulObject = ConcurrentLoggedStatusfulObject<WFStatus,WFStatus::OK>;

    template <typename T>
    using WFResult = StatusfulResult<T,WFStatus,WFStatus::OK>;

    using WFStatusResult = StatusResult<WFStatus,WFStatus::OK>;
}