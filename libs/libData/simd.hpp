// Copyright (c) 2022 Tom Hancocks
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <algorithm>

namespace data
{
    class block;
}

namespace data::simd
{

#if __x86_64__
    typedef          __int128    int128_t;
    typedef unsigned __int128   uint128_t;

    typedef uint128_t           wide_type;
    typedef wide_type           type;
    typedef uint64_t            wide_field_type;

    static constexpr std::size_t alignment_width = 16;
    static constexpr std::size_t field_count = 4;
    static constexpr std::size_t wide_field_count = 2;

#elif __arm64__
    typedef uint64_t            wide_type;
    typedef wide_type           type;
    typedef type                wide_field_type;

    static constexpr std::size_t alignment_width = 8;
    static constexpr std::size_t field_count = 2;
    static constexpr std::size_t wide_field_count = 1;

#else
    typedef uint32_t            simd_wide_type;
    typedef simd_wide_type      simd_type;

    static constexpr std::size_t alignment_width = 4;
    static constexpr std::size_t field_count = 1;
    static constexpr std::size_t wide_field_count = 0;

#endif

    typedef uint32_t             field_type;
    typedef uint16_t             half_field_type;
    static constexpr std::size_t field_size = sizeof(field_type);
    static constexpr std::size_t fields_length = field_count * field_size;

    union alignas(alignment_width) value
    {
        type wide;
#if __x86_64__ || __arm64__
        wide_field_type wide_fields[wide_field_count];
#endif
        field_type fields[field_count];
        uint8_t bytes[sizeof(type)];
    };

    static constexpr std::size_t alignment_mask = ~(alignment_width - 1);

    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
    static inline auto expand_capacity(T capacity) -> T
    {
        return (capacity + alignment_width - 1) & alignment_mask;
    }

    template<typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
    static inline auto align(T ptr) -> T
    {
        return reinterpret_cast<T>((reinterpret_cast<std::uintptr_t>(ptr) + alignment_width - 1) & alignment_mask);
    }

    __attribute__((optnone)) static inline auto set(std::uint32_t *ptr, std::size_t dst_size, union value v, std::size_t bytes) -> void
    {
        std::size_t n = 0;
        auto len = std::min(dst_size, bytes);
        while (n < len) {
            if ((reinterpret_cast<uintptr_t>(ptr) & alignment_width) || (len - n) < fields_length) {
                *ptr = v.fields[n & (field_count - 1)];
                ++ptr;
                n += field_size;
            }
            else {
                *reinterpret_cast<wide_type *>(ptr) = v.wide;
                ptr += field_count;
                n += alignment_width;
            }
        }
    }

}