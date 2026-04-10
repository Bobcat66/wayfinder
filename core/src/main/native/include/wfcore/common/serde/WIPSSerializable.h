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

// TODO: refactor shims to use WIPSSerializable-based design rather than serde.h
#pragma once

#include "wips/wips_runtime.h"
#include "wfcore/common/status.h"
#include <cstdint>

namespace wf {

    // Every derived type of WIPSSerializable
    // MUST implement public methods with the following signatures:
    // `static WipsType toWIPS_impl(const DerivedType&)`
    // `static DerivedType fromWIPS_impl(const WipsType&)`
    template <typename DerivedType,typename WipsType,wips_voidmethods_t* WipsMethods>
    class WIPSSerializable {
    public:
        static WipsType toWIPS(const DerivedType& wfcore_object) {
            return DerivedType::toWIPS_impl(wfcore_object);
        }

        static DerivedType fromWIPS(const WipsType& wips_struct) {
            return DerivedType::fromWIPS_impl(wips_struct);
        }

        static wips_blob_t* pack(const DerivedType& wfcore_object) {
            auto wipsStruct = DerivedType::toWIPS_impl(wfcore_object);
            wips_blob_t* blob = wips_blob_create(sizeof(WipsType));
            WipsMethods->encode(blob, (void*)&wipsStruct);
            WipsMethods->free_resources((void*)&wipsStruct);
            return blob;
        }

        static DerivedType unpack(wips_blob_t* data) {
            WipsType wipsStruct;
            WipsMethods->decode((void*)&wipsStruct, data);
            auto out = DerivedType::fromWIPS_impl(wipsStruct);
            WipsMethods->free_resources((void*)&wipsStruct);
            return out;
        }
    };

}