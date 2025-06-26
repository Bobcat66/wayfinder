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

#include <vector>
#include <cstdint>
#include <type_traits>
#include <concepts>
#include <cstring> 
#include <cassert>

namespace wf {

    using byte = uint8_t;

    template<typename T>
    concept TriviallyCopyable = std::is_trivially_copyable_v<T>;

    template<TriviallyCopyable T>
    inline size_t packTrivial(std::vector<byte>& out,const T& data) {
        constexpr size_t tsize = sizeof(T);
        const byte* ptr = reinterpret_cast<const byte*>(&data);
        out.insert(out.end(), ptr, ptr + tsize);
        return static_cast<int>(tsize);
    }

    // Reads into out from a vector, beginning at the iterator.
    template<TriviallyCopyable T>
    inline T unpackTrivial(const std::vector<byte>& data, std::vector<byte>::const_iterator& cit) {
        constexpr size_t tsize = sizeof(T);
        assert(std::distance(cit, data.end()) >= static_cast<std::ptrdiff_t>(tsize));
        T out;
        std::memcpy(&out,&*cit,tsize);
        cit += tsize;
        return out;
    }
}