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