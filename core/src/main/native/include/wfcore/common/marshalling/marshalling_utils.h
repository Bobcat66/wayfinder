#pragma once

#include <vector>
#include <cstdint>
#include <type_traits>
#include <concepts>

namespace wf {

    using byte = uint8_t;

    template<typename T>
    concept TriviallyCopyable = std::is_trivially_copyable_v<T>;

    template<TriviallyCopyable T>
    inline int marshalTrivial(std::vector<byte>& out,const T& data) {
        constexpr size_t tsize = sizeof(T);
        const byte* ptr = reinterpret_cast<const byte*>(&data);
        out.insert(out.end(), ptr, ptr + tsize);
        return tsize;
    }
}