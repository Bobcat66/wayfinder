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

#include "wfcore/common/jval_compat.h"

namespace impl {
    using namespace wf;
    static constexpr inline WFStatus JVStatusToWF(jval::JVStatus status) noexcept {
        switch (status) {
            case jval::JVStatus::OK: return WFStatus::OK;
            case jval::JVStatus::MISSING_PROPERTY: return WFStatus::JSON_PROPERTY_NOT_FOUND;
            case jval::JVStatus::BAD_TYPE: return WFStatus::JSON_INVALID_TYPE;
            case jval::JVStatus::SCHEMA_VIOLATION: return WFStatus::JSON_SCHEMA_VIOLATION;
            case jval::JVStatus::PARSE_ERROR: return WFStatus::JSON_PARSE;
            case jval::JVStatus::UNKNOWN: return WFStatus::JSON_UNKNOWN;
            default: return WFStatus::UNKNOWN;
        }
    }
}

namespace wf {
    WFStatusResult JVResToWF(const jval::JVResult& result) {
        if (result.ok()) return WFStatusResult::success();
        auto wf_status = impl::JVStatusToWF(result.status());
        return WFStatusResult::failure(wf_status,result.what());
    }
}