// Copyright (c) 2023 Tom Hancocks
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

#include <cstring>
#include <cstdint>
#include <type_traits>
#include <libSIMD/SIMD.hpp>

namespace simd::string
{

    template<typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
    inline auto copy(T dst, const T src, std::size_t n) -> T
    {
#if !USE_TARGET_MEMORY_FUNCTIONS
        // TODO: Implement
        return src;
#else
        return reinterpret_cast<T>(::memcpy(dst, src, n));
#endif
    }

    template<typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
    inline auto set(T ptr, std::uint8_t v, std::size_t n) -> void
    {
#if !USE_TARGET_MEMORY_FUNCTIONS
        // TODO: Implement
#else
        ::memset(ptr, v, n);
#endif
    }

    template<typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
    inline auto setw(T ptr, std::uint16_t v, std::int32_t n) -> void
    {
        // TODO: Implement
    }

    template<typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
    inline auto setl(T ptr, std::uint32_t v, std::int32_t n) -> void
    {
        // TODO: Implement a better version of this.
        auto dst = reinterpret_cast<std::uint32_t *>(ptr);
        auto len = n >> 2;
        auto rem = n - (len << 2);

        while (len--) {
            *dst++ = v;
        }

        if (rem) {
            auto dst_byte = reinterpret_cast<std::uint8_t *>(dst);
            while (rem--) {
                *dst_byte++ = (v >> (8 * rem)) & 0xFF;
            }
        }
    }

    template<typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
    inline auto setq(T ptr, std::uint64_t v, std::int32_t n) -> void
    {
        // TODO: Implement
    }

}