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

namespace wf {

    // Every derived type of WIPSSerializable
    // MUST implement public methods with the following signatures:
    // `static DerivedType wips2wfcore_shim_impl(const WipsType&)`
    // `static WipsType wips2wfcore_shim_impl(const DerivedType&)`
    // `static wips_bin_t* pack_impl(const DerivedType&)`
    // `static DerivedType unpack_impl(wips_bin_t*)`
    template <typename DerivedType,typename WipsType>
    class WIPSSerializable {
    public:
        static DerivedType wips2wfcore_shim(const WipsType& wips_struct) {
            return DerivedType::wips2wfcore_shim_impl(wips_struct);
        }

        static WipsType wfcore2wips_shim(const DerivedType& wfcore_object) {
            return DerivedType::wfcore2wips_shim_impl(wfcore_object);
        }

        static wips_bin_t* pack(const DerivedType& wfcore_object) {
            return DerivedType::pack_impl(wfcore_object);
        }

        static DerivedType unpack(wips_bin_t* data) {
            return DerivedType::unpack_impl(data);
        }
    };

}